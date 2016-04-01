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
	_indexDirectory(true), WSSession(peerAddress, file, protocol, invoker), _writer(*this),
	_decoder(invoker), onDecoded([this](const std::shared_ptr<HTTPPacket>& pPacket, const SocketAddress& address) {receive(pPacket); }), onDecodedEnd([this]() {flush(); }),
	onCallProperties([this](DataReader& reader,string& value) {
		HTTP::OnCookie onCookie([this,&value](const char* key, const char* data, UInt32 size) {
			peer.properties().setString(key, data, size);
			value.assign(data,size);
		});
		return _writer.writeSetCookie(reader, onCookie);
	}) {

	peer.OnCallProperties::subscribe(onCallProperties); // subscribe to client.properties(...)

	_decoder.OnDecodedEnd::subscribe(onDecodedEnd);
	_decoder.OnDecoded::subscribe(onDecoded);
}

const char* HTTPSession::cookie(const char* key) {
	const HTTPPacket* pPacket = _writer.lastRequest();
	if (!pPacket)
		return NULL;
	auto it(pPacket->cookies.find(key));
	return it == pPacket->cookies.end() ? NULL : it->second.c_str();
}

void HTTPSession::kill(UInt32 type){
	if(died)
		return;

	peer.OnCallProperties::unsubscribe(onCallProperties);

	closePublication();
	closeSusbcription();

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

	_writer.beginRequest(pPacket);

	// HTTP is a simplex communication, so if request, remove possible old subscription
	closePublication();
	closeSusbcription();

	//// Disconnection if path has changed
	if(peer.connected && (pPacket->connection&HTTP::CONNECTION_UPDATE || String::ICompare(peer.path,pPacket->path)!=0 || String::ICompare(peer.query,pPacket->query)!=0)) // if path change or query!! (onConnection grab query!)
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
	PacketReader query(BIN peer.query.data(), peer.query.size());
	QueryReader parameters(query);
	
	Exception ex;

	// Upgrade protocol
	if(pPacket->connection&HTTP::CONNECTION_UPGRADE) {
		// Upgrade to WebSocket
		if (String::ICompare(pPacket->upgrade,"websocket")==0) {
			peer.onDisconnection();
			WSSession::enable(); // keep before onConnection to solve peer.protocol value (and parameters)

			DataWriter& response = _writer.writeRaw("101 Switching Protocols");
			HTTP_BEGIN_HEADER(response.packet)
				HTTP_ADD_HEADER("Upgrade",EXPAND("WebSocket"))
				HTTP_ADD_HEADER("Sec-WebSocket-Accept", WS::ComputeKey(pPacket->secWebsocketKey))
			HTTP_END_HEADER
			peer.onConnection(ex, wsWriter(),parameters,DataWriter::Null); // No response in handshake
		} // TODO else
	} else {
		
		// onConnection
		if (!peer.connected) {

			_index.clear(); // default value
			_indexDirectory = true; // default value

			peer.onConnection(ex, _writer,parameters);
			if (!ex && peer.connected) {
				if (peer.parameters().getString("index", _index)) {
					if (String::IsFalse(_index)) {
						_indexDirectory = false;
						_index.clear();
					} else if (String::IsTrue(_index))
						_index.clear();
					else
						FileSystem::GetName(_index); // Redirect to the file (get name to prevent path insertion)
				}

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
				if (!pPacket->contentType || !MIME::CreateDataReader(MIME::DataType(pPacket->contentSubType.c_str()), packet, invoker.poolBuffers, pReader))
					pReader.reset(new StringReader(packet));
				readMessage(ex,*pReader);
			}
			//  HTTP OPTIONS (it is due requested when Move Redirection is sent)
			else if (pPacket->command == HTTP::COMMAND_OPTIONS)
				processOptions(ex, *pPacket);
			else
				ex.set(Exception::PROTOCOL, "Unsupported command");
		}
	}

	if (ex)
		_writer.close(ex);
	else if(!peer.connected)
		kill(REJECTED_DEATH);

	_writer.endRequest();
}

void HTTPSession::manage() {
	if(WSSession::enabled())
		return WSSession::manage();
	TCPSession::manage();
}

void HTTPSession::processOptions(Exception& ex,const HTTPPacket& request) {

	// Control methods quiested
	if (request.accessControlRequestMethod&HTTP::COMMAND_DELETE) {
		ex.set(Exception::PROTOCOL,"Delete not allowed");
		return;
	}

	HTTP_BEGIN_HEADER(_writer.writeRaw("200 OK").packet)
		HTTP_ADD_HEADER("Access-Control-Allow-Methods", EXPAND("GET, HEAD, PUT, PATH, POST, OPTIONS"))
		if (request.accessControlRequestHeaders)
			HTTP_ADD_HEADER("Access-Control-Allow-Headers", request.accessControlRequestHeaders)
		HTTP_ADD_HEADER("Access-Control-Max-Age", EXPAND("86400")) // max age of 24 hours
	HTTP_END_HEADER
}

void HTTPSession::processGet(Exception& ex, HTTPPacket& request, QueryReader& parameters) {
	// use index http option in the case of GET request on a directory

	File& file(request.file);

	if (!file.isFolder()) {
		// FILE //

		// 1 - priority on client method
		if (file.extension().empty() && peer.onMessage(ex, file.name(), parameters)) // can be method!
			return;
	
		// 2 - try to get a file
		shared_ptr<Parameters> pFileParams(new MapParameters());
		ParameterWriter parameterWriter(*pFileParams);
		if (!peer.onRead(ex, parameters, file, parameterWriter)) {
			if (!ex)
				ex.set(Exception::PERMISSION, "No authorization to see the content of ", file.path());
			return;
		}
		// If onRead has been authorised, and that the file is a multimedia file, and it doesn't exists (no VOD, filePath.lastModified()==0 means "doesn't exists")
		// Subscribe for a live stream with the basename file as stream name
		if (!file.exists()) {
			if (request.contentType == HTTP::CONTENT_ABSENT)
				request.contentType = HTTP::ExtensionToMIMEType(file.extension(), request.contentSubType);
			if (request.contentType == HTTP::CONTENT_VIDEO || request.contentType == HTTP::CONTENT_AUDIO) {
				if (openSubscribtion(ex, file.baseName(), _writer))
					return;
			}
		}
		if (!file.isFolder())
			return _writer.writeFile(file.path(), pFileParams);
	}


	// FOLDER //
	if (_index.find_last_of('.') == string::npos && peer.onMessage(ex, _index, parameters)) // can be method!
		return;

	// Redirect to the file (get name to prevent path insertion), if impossible (_index empty or invalid) list directory
	if (!_index.empty())
		file.setPath(file.path(),_index);
	else if (!_indexDirectory) {
		ex.set(Exception::PERMISSION, "No authorization to see the content of ", peer.path, '/');
		return;
	}
	shared_ptr<Parameters> pFileParams(new MapParameters());
	ParameterWriter parameterWriter(*pFileParams);
	parameters.read(parameterWriter);
	return _writer.writeFile(file.path(), pFileParams); // folder view or index redirection (without pass by onRead because can create a infinite loop
}

} // namespace Mona
