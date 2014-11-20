/*
Copyright 2014 Mona
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License received along this program for more
details (or else see http://www.gnu.org/licenses/).

This file is a part of Mona.
*/

#include "Mona/HTTP/HTTPSession.h"
#include "Mona/HTTP/HTTP.h"
#include "Mona/ParameterWriter.h"
#include "Mona/StringReader.h"
#include "Mona/QueryReader.h"
#include "Mona/Protocol.h"
#include "Mona/Exceptions.h"
#include "Mona/FileSystem.h"
#include "Mona/MIME.h"


using namespace std;


namespace Mona {


HTTPSession::HTTPSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker) :
	_indexDirectory(true),WSSession(peerAddress, file, protocol, invoker), _writer(*this), _pListener(NULL),
	_decoder(invoker),onDecoded([this](const std::shared_ptr<HTTPPacket>& pPacket,const SocketAddress& address){receive(pPacket);}), onDecodedEnd([this](){flush();}) {

	onCallProperties = [this](vector<string>& items) {
		UInt8 count = items.size();

		HTTPPacket* pRequest(_writer.request());
		if(!pRequest)
			ERROR("HTTPSession ",name()," process cookies without upstream request")
		else if (!count) {
			ERROR("HTTPSession ",name(),", cookie's key argument missing")
			return false;
		}

		if (count == 1) {
			// READ COOKIE
			auto it(pRequest->cookies.find(items[0]));
			if (it == pRequest->cookies.end())
				return false;
			items.clear();
			items.emplace_back(it->second);
			return true;
		}

		// WRITE COOKIE

		string& setCookie(*pRequest->sendingInfos().setCookies.emplace(pRequest->sendingInfos().setCookies.end(), items[0]));
		string value(items[1]);
		String::Append(setCookie, "=", value);

		// Expiration Date in RFC 1123 Format
		if (count > 2) {

			Int32 expiresOffset = 0;
			if (String::ToNumber<Int32>(items[2], expiresOffset)) {
				Date expiration(Date::Type::GMT);
				expiration += expiresOffset*1000; // Now + signed offset
				string dateExpiration;
				String::Append(setCookie, "; Expires=", expiration.toString(Date::RFC1123_FORMAT, dateExpiration));
			}
		}

		if (count > 3 && !items[3].empty()) String::Append(setCookie, "; Path=", items[3]);
		if (count > 4 && !items[4].empty()) String::Append(setCookie, "; Domain=", items[4]);
		if (count > 5 && items[5] == "true") String::Append(setCookie, "; Secure");
		if (count > 6 && items[6] == "true") String::Append(setCookie, "; HttpOnly");

		// Return value from key added
		items.clear();
		items.emplace_back(value);
		// Add new cookie to cookies
		pRequest->cookies[items[0]].assign(value);
		return true;
	};


	peer.OnCallProperties::subscribe(onCallProperties); // subscribe to client.properties(...)

	_decoder.OnDecodedEnd::subscribe(onDecodedEnd);
	_decoder.OnDecoded::subscribe(onDecoded);
}

void HTTPSession::kill(UInt32 type){
	if(died)
		return;

	peer.OnCallProperties::unsubscribe(onCallProperties);

	if (_pListener) {
		invoker.unsubscribe(peer, _pListener->publication.name());
		_pListener = NULL;
	}
	if (WSSession::enabled())
		WSSession::kill(type);
	else
		TCPSession::kill(type);

	// no more reception
	_decoder.OnDecoded::unsubscribe(onDecoded);
	_decoder.OnDecodedEnd::unsubscribe(onDecodedEnd);
}

void HTTPSession::receive(const shared_ptr<HTTPPacket>& pPacket) {

	if (!TCPSession::receive(*pPacket))
		return;

	// invalid packet?
	
	if (pPacket->exception)
		return _writer.close(pPacket->exception);

	_writer.setRequest(pPacket);

	// HTTP is a simplex communication, so if request, remove possible old subscription
	if (_pListener) {
		invoker.unsubscribe(peer, _pListener->publication.name());
		_pListener = NULL;
	}

	//// Disconnection if path has changed
	if(peer.connected && String::ICompare(peer.path,pPacket->path)!=0)
		peer.onDisconnection();

	////  Fill peers infos
	peer.setPath(pPacket->path);
	peer.setQuery(pPacket->query);
	peer.setServerAddress(pPacket->serverAddress);
	// write properties (headers + cookies)
	peer.properties().clear();
	peer.properties().setNumber("version", pPacket->version);
	for (auto it : pPacket->headers)
		peer.properties().setString(it.first, it.second);
	for (auto it : pPacket->cookies)
		peer.properties().setString(it.first, it.second);

	// Create parameters for onConnection or a GET onRead/onWrite/onMessage
	QueryReader parameters(peer.query.c_str());
	
	Exception ex;

	// Upgrade protocol
	if(pPacket->connection&HTTP::CONNECTION_UPGRADE) {
		// Upgrade to WebSocket
		if (String::ICompare(pPacket->upgrade,"websocket")==0) {
			peer.onDisconnection();
			WSSession::enable();

			DataWriter& response = _writer.write("101 Switching Protocols");
			HTTP_BEGIN_HEADER(response.packet)
				HTTP_ADD_HEADER("Upgrade",EXPAND("WebSocket"))
				HTTP_ADD_HEADER("Sec-WebSocket-Accept", WS::ComputeKey(pPacket->secWebsocketKey))
			HTTP_END_HEADER
			peer.onConnection(ex, wsWriter(),parameters,response);
		} // TODO else
	} else {
		
		// onConnection
		if (!peer.connected) {

			_index.clear(); // default value
			_indexDirectory = true; // default value

			peer.onConnection(ex, _writer,parameters);
			if (!ex && peer.connected) {

				if (!peer.parameters().getBool("index", _indexDirectory))
					peer.parameters().getString("index", _index);

				parameters.reset();
			}
		}

		// onMessage/<method>/onRead
		if (!ex && peer.connected) {

			/// HTTP GET & HEAD
			if ((pPacket->command == HTTP::COMMAND_HEAD ||pPacket->command == HTTP::COMMAND_GET))
				processGet(ex, *pPacket, parameters);
			// HTTP POST
			else if (pPacket->command == HTTP::COMMAND_POST) {
				PacketReader packet(pPacket->content, pPacket->contentLength);
				unique_ptr<DataReader> pReader;
				if (!MIME::CreateDataReader(MIME::DataType(pPacket->contentSubType.c_str()), packet, invoker.poolBuffers, pReader)) {
					pReader.reset(new StringReader(packet));
					pPacket->rawSerialization = true;
				}
				readMessage(ex,*pReader);
			}
			//  HTTP OPTIONS (it is due requested when Move Redirection is sent)
			else if (pPacket->command == HTTP::COMMAND_OPTIONS)
				processOptions(ex, pPacket->sendingInfos());
			else
				ex.set(Exception::PROTOCOL, "Unsupported command");
		}
	}

	if (ex)
		_writer.close(ex);
	else if(!peer.connected)
		kill(REJECTED_DEATH);

	_writer.flush(true); // flush with push, like that if nothing has been answered it means that the request is here to fill the pushSenders list
}

void HTTPSession::manage() {
	if(WSSession::enabled())
		return WSSession::manage();
	TCPSession::manage();
}

void HTTPSession::processOptions(Exception& ex,const HTTPSendingInfos& infos) {

	// Control methods quiested
	if (infos.accessControlRequestMethod&HTTP::COMMAND_DELETE) {
		ex.set(Exception::PROTOCOL,"Delete not allowed");
		return;
	}

	HTTP_BEGIN_HEADER(_writer.write("200 OK").packet)
		HTTP_ADD_HEADER("Access-Control-Allow-Methods", EXPAND("GET, HEAD, PUT, PATH, POST, OPTIONS"))
		if (!infos.accessControlRequestHeaders.empty())
			HTTP_ADD_HEADER("Access-Control-Allow-Headers", infos.accessControlRequestHeaders)
	HTTP_END_HEADER
}

void HTTPSession::processGet(Exception& ex, HTTPPacket& packet, QueryReader& parameters) {
	// use index http option in the case of GET request on a directory

	Path file(packet.filePath());

	if (file.isFolder()) {
		// FOLDER //
		if (!_index.empty()) {
			if (_index.find_last_of('.') != string::npos && peer.onMessage(ex, _index, parameters)) // can be method!
				return;
			// Redirect to the file (get name to prevent path insertion)
			file.append('/', FileSystem::GetName(_index)); 
			return processGet(ex, packet, parameters);
		}
		
		if (_indexDirectory)
			_writer.writeFile(file, parameters); // folder view!
		else
			ex.set(Exception::PERMISSION, "No authorization to see the content of ", peer.path, "/");
	} else {
		// FILE //

		// 1 - priority on client method
		if (file.extension().empty() && peer.onMessage(ex, file.name(), parameters)) // can be method!
			return;
	
		// 2 - try to get a file
		ParameterWriter parameterWriter(packet.sendingInfos().parameters);
		if (peer.onRead(ex, file, parameters, parameterWriter) && !ex) {
			packet.sendingInfos().sizeParameters = parameterWriter.size();
			
			// If onRead has been authorised, and that the file is a multimedia file, and it doesn't exists (no VOD, filePath.lastModified()==0 means "doesn't exists")
			// Subscribe for a live stream with the basename file as stream name
			if (!file.exists()) {
				if (packet.contentType == HTTP::CONTENT_ABSENT)
					packet.contentType = HTTP::ExtensionToMIMEType(file.extension(), packet.contentSubType);
				if (packet.contentType == HTTP::CONTENT_VIDEO || packet.contentType == HTTP::CONTENT_AUDIO) {
					if (_pListener = invoker.subscribe(ex, peer, file.baseName(), _writer))
						return;
				}
			}
			parameters.reset();
			_writer.writeFile(file, parameters);
		}
	}
}

} // namespace Mona
