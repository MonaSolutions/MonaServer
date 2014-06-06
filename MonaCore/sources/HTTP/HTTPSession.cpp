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
#include "Mona/HTTP/HTTPPacketBuilding.h"
#include "Mona/HTTPHeaderReader.h"
#include "Mona/ParameterWriter.h"
#include "Mona/SOAPReader.h"
#include "Mona/SOAPWriter.h"
#include "Mona/Protocol.h"
#include "Mona/Exceptions.h"
#include "Mona/FileSystem.h"


using namespace std;


namespace Mona {


HTTPSession::HTTPSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker) : _indexCanBeMethod(false),_indexDirectory(true),WSSession(peerAddress, file, protocol, invoker), _isWS(false), _writer(*this),_ppBuffer(new PoolBuffer(invoker.poolBuffers)), _pListener(NULL),
	onCallProperties([this](vector<string>& items) {
		UInt8 count = items.size();

		if(!_writer.pRequest)
			ERROR("HTTPSession ",name()," process cookies without upstream request")
		else if (!count)
			ERROR("HTTPSession ",name(),", cookie's key argument missing")
		else if (count == 1)
			ERROR("HTTPSession ",name(),", ",items[0]," cookie's value argument missing")
		else {
			string& setCookie(*_writer.pRequest->sendingInfos().setCookies.emplace(_writer.pRequest->sendingInfos().setCookies.end(), items[0]));
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
		}
		return false;
	}) {
}

void HTTPSession::kill(UInt32 type){
	if(died)
		return;

	peer.OnCallProperties::unsubscribe(onCallProperties);

	if (_pListener) {
		invoker.unsubscribe(peer, _pListener->publication.name());
		_pListener = NULL;
	}
	if (_isWS)
		WSSession::kill(type);
	else
		TCPSession::kill(type);
}

bool HTTPSession::buildPacket(PoolBuffer& pBuffer,PacketReader& packet) {
	if(_isWS)
		return WSSession::buildPacket(pBuffer,packet);
	// consumes all!
	_packets.emplace_back(decode<HTTPPacketBuilding>(pBuffer,_ppBuffer).pPacket);
	return true;
}	

void HTTPSession::packetHandler(PacketReader& reader) {

	// WSSession?
	if(_isWS)
		return WSSession::packetHandler(reader);

	// Pop HTTPPacket (must be done just one time per handle!)
	if (!_packets.empty()) {
		_writer.pRequest = _packets.front();
		_packets.pop_front();
	}
	const shared_ptr<HTTPPacket>& pPacket(_writer.pRequest);
	if (!pPacket) {
		ERROR("HTTPSession::packetHandler without http packet built");
		return;
	}

	if (pPacket->exception)
		return _writer.close(pPacket->exception);

	// HTTP is a simplex communication, so if request, remove possible old subscription
	if (_pListener) {
		invoker.unsubscribe(peer, _pListener->publication.name());
		_pListener = NULL;
	}

	// Rember full path and extract directory part
	_filePath.set(pPacket->path);
	if (pPacket->filePos != string::npos)
		((string&)pPacket->path).erase(pPacket->filePos - 1);

	//// Disconnection if path has changed
	if(peer.connected && String::ICompare(peer.path,pPacket->path)!=0)
		peer.onDisconnection();

	////  fill peers infos
	peer.setPath(pPacket->path);
	peer.setQuery(pPacket->query);
	peer.setServerAddress(pPacket->serverAddress);
	// erase previous properties
	peer.properties().clear();
	peer.properties().setNumber("HTTPVersion", pPacket->version); // TODO check how is named for AMF
	// Add cookies to peer.properties
	String::ForEach forEachCookie([this](const char* key) {
		const char* value = key;
		// trim right
		while (value && *value != '=' && !isblank(*value))
			++value;
		if (value) {
			*(char*)value = '\0';
			// trim left
			do {
				++value;
			} while (value && (isblank(*value) || *value == '='));
		}
		peer.properties().setString(key, value);
	});
	String::Split(pPacket->cookies, ";", forEachCookie, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
	
	Exception ex;
	// Upgrade protocol
	if(pPacket->connection&HTTP::CONNECTION_UPGRADE) {
		// Upgrade to WebSocket
		if (String::ICompare(pPacket->upgrade,"websocket")==0) {
			peer.onDisconnection();
			_isWS=true;
			((string&)this->peer.protocol) = "WebSocket";
			
			DataWriter& response = _writer.write("101 Switching Protocols", HTTP::CONTENT_ABSENT);
			BinaryWriter& writer = response.packet;
			HTTP_BEGIN_HEADER(writer)
				HTTP_ADD_HEADER(writer,"Upgrade","WebSocket")
				HTTP_ADD_HEADER(writer,"Sec-WebSocket-Accept", WS::ComputeKey(pPacket->secWebsocketKey))
			HTTP_END_HEADER(writer)
			HTTPHeaderReader reader(pPacket->headers);
			peer.onConnection(ex, wsWriter(),reader,response);
			_writer.flush(true); // last HTTP flush for this connection, now we are in a WebSession mode!
		} // TODO else
	} else {
		// Create volatile parameters from POST/GET/HEAD Queries
		MapParameters queryParameters;
		Util::UnpackQuery(peer.query, queryParameters);
		MapReader<MapParameters> propertiesReader(queryParameters);
		// TODO add the content of post request as volatile parameters in addition of before (SplitReader + TypeReader)

		// onConnection
		if (!peer.connected) {
			// Assign name
			string buffer;
			queryParameters.getString("name", buffer);
			peer.properties().setString("name", buffer);

			peer.OnCallProperties::subscribe(onCallProperties); // subscribe to client.properties(...)

			peer.onConnection(ex, _writer,propertiesReader);
			if (!ex && peer.connected) {
				peer.parameters().getBool("index", _indexDirectory);
				peer.parameters().getString("index", _index);
				if(!_indexDirectory)
					_indexCanBeMethod = _index.find_last_of('.')==string::npos;
				propertiesReader.reset();
			}
		}

		// onMessage/<method>/onRead
		if (!ex && peer.connected) {

			/// HTTP GET & HEAD
			if ((pPacket->command == HTTP::COMMAND_HEAD ||pPacket->command == HTTP::COMMAND_GET))
				processGet(ex, pPacket, propertiesReader);
			// HTTP POST
			else if (pPacket->command == HTTP::COMMAND_POST) {
				PacketReader packetContent(pPacket->content, pPacket->contentLength);

				// Get output format from input
				_writer.contentType = pPacket->contentType;
				_writer.contentSubType = pPacket->contentSubType;

				HTTP::ReadMessageFromType(ex, *this, pPacket, packetContent);
			}
			//  HTTP OPTIONS (it is due requested when Move Redirection is sent)
			else if (pPacket->command == HTTP::COMMAND_OPTIONS)
				processOptions(ex, pPacket);
			else
				ex.set(Exception::PROTOCOL, "Unsupported command");
		}
	}

	if (ex)
		_writer.close(ex);
	else if(!peer.connected)
		kill(REJECTED_DEATH);
}

void HTTPSession::manage() {
	if(_isWS) {
		WSSession::manage();
		return;
	}
	if (!_packets.empty() && _packets.front()->exception)
		_writer.close(_packets.front()->exception);
	TCPSession::manage();
}

void HTTPSession::processOptions(Exception& ex, const shared_ptr<HTTPPacket>& pPacket) {

	// Control methods quiested
	if (pPacket->accessControlRequestMethod&HTTP::COMMAND_DELETE) {
		ex.set(Exception::PROTOCOL,"Delete not allowed");
		return;
	}

	DataWriter& response = _writer.write("200 OK", HTTP::CONTENT_ABSENT);
	BinaryWriter& writer = response.packet;

	HTTP_BEGIN_HEADER(writer)
		HTTP_ADD_HEADER(writer,"Access-Control-Allow-Origin", "*")
		HTTP_ADD_HEADER(writer,"Access-Control-Allow-Methods", "GET, HEAD, PUT, PATH, POST, OPTIONS")
		HTTP_ADD_HEADER(writer,"Access-Control-Allow-Headers", "Content-Type")
	HTTP_END_HEADER(writer)
}

void HTTPSession::processGet(Exception& ex, const shared_ptr<HTTPPacket>& pPacket, MapReader<MapParameters>& propertiesReader) {
	// use index http option in the case of GET request on a directory
	bool methodCalled(false);

	// 1 - priority on client method
	if (pPacket->filePos == string::npos) {
		if (!_index.empty()) {
			if (_indexCanBeMethod)
				methodCalled = peer.onMessage(ex, _index, propertiesReader);

			if (!methodCalled && !ex) {
				// Redirect to the file (get name to prevent path insertion)
				string nameFile;
				_filePath.appendPath('/', FileSystem::GetName(_index, nameFile));
			}
		} else if (!_indexDirectory)
			ex.set(Exception::PERMISSION, "No authorization to see the content of ", peer.path, "/");
	} else
		methodCalled = peer.onMessage(ex, _filePath.name(), propertiesReader);

	// 2 - try to get a file
	if (!methodCalled && !ex) {
		ParameterWriter parameterWriter(pPacket->sendingInfos().parameters);
		if (peer.onRead(ex, _filePath, propertiesReader, parameterWriter) && !ex) {
			pPacket->sendingInfos().sizeParameters = parameterWriter.size();
			// If onRead has been authorised, and that the file is a multimedia file, and it doesn't exists (no VOD, filePath.lastModified()==0 means "doesn't exists")
			// Subscribe for a live stream with the basename file as stream name
			if (_filePath.lastModified() == 0) {
				if (pPacket->contentType == HTTP::CONTENT_ABSENT)
					pPacket->contentType = HTTP::ExtensionToMIMEType(_filePath.extension(),pPacket->contentSubType);
				if ((pPacket->contentType == HTTP::CONTENT_VIDEO || pPacket->contentType == HTTP::CONTENT_AUDIO) && _filePath.lastModified() == 0)
					_pListener = invoker.subscribe(ex, peer, _filePath.baseName(), _writer);
			}
			if (!ex && !_pListener) {
					// for the case of one folder displayed, search sort arguments
				string direction;
				UInt8 sortOptions(HTTP::SORT_ASC);
					if (peer.properties().getString("N", direction))
					sortOptions |= HTTP::SORT_BY_NAME;
					else if (peer.properties().getString("M", direction))
					sortOptions |= HTTP::SORT_BY_MODIFIED;
					else if (peer.properties().getString("S", direction))
					sortOptions |= HTTP::SORT_BY_SIZE;
					if (direction == "D")
						sortOptions |= HTTP::SORT_DESC;
					// HTTP get
				_writer.writeFile(_filePath, sortOptions, pPacket->filePos==string::npos);
			}
		}
	}
}

} // namespace Mona
