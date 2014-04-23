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
#include "Mona/HTTPHeaderReader.h"
#include "Mona/SOAPReader.h"
#include "Mona/SOAPWriter.h"
#include "Mona/Protocol.h"
#include "Mona/Exceptions.h"
#include "Mona/FileSystem.h"
#include "Mona/HTTP/HTTPPacketBuilding.h"


using namespace std;


namespace Mona {


HTTPSession::HTTPSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker) : WSSession(peerAddress, file, protocol, invoker), _isWS(false), _writer(*this),_ppBuffer(new PoolBuffer(invoker.poolBuffers)), _pListener(NULL) {

}

void HTTPSession::kill(UInt32 type){
	if(died)
		return;
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
	shared_ptr<HTTPPacketBuilding> pHTTPPacketBuilding(new HTTPPacketBuilding(invoker,pBuffer,_ppBuffer));
	_packets.emplace_back(pHTTPPacketBuilding->pPacket);
	decode<HTTPPacketBuilding>(pHTTPPacketBuilding);
	return true;
}

const shared_ptr<HTTPPacket>& HTTPSession::packet() {
	if (_packets.empty())
		return _writer.pRequest;
	_writer.pRequest = _packets.front();
	_packets.pop_front();
	return _writer.pRequest;
}
	

void HTTPSession::packetHandler(PacketReader& reader) {
	if(_isWS) {
		WSSession::packetHandler(reader);
		return;
	}

	const shared_ptr<HTTPPacket>& pPacket(packet());
	if (!pPacket) {
		ERROR("HTTPSession::packetHandler without http packet built");
		return;
	}

	if (pPacket->exception) {
		_writer.close(pPacket->exception);
		return;
	}

	// HTTP is a simplex communication, so if request, remove possible old subscription
	if (_pListener) {
		invoker.unsubscribe(peer, _pListener->publication.name());
		_pListener = NULL;
	}

	
	FilePath filePath(pPacket->path);
	if (pPacket->filePos != string::npos)
		((string&)pPacket->path).erase(pPacket->filePos - 1);

	//// Disconnection if path has changed
	if(peer.connected && String::ICompare(peer.path,pPacket->path)!=0)
		peer.onDisconnection();

	////  fill peers infos
	peer.setPath(pPacket->path);
	peer.setQuery(pPacket->query);
	peer.setServerAddress(pPacket->serverAddress);
	// erase previous parameters
	peer.properties().clear();
	peer.properties().setNumber("HTTPVersion", pPacket->version); // TODO check how is named for AMF
	Util::UnpackQuery(peer.query,peer.properties());
	
	/// Client onConnection
	Exception ex;
	if(pPacket->connection&HTTP::CONNECTION_UPGRADE) {
		// Ugrade to WebSocket
		if (String::ICompare(pPacket->upgrade,"websocket")==0) {
			peer.onDisconnection();
			_isWS=true;
			((string&)this->peer.protocol) = "WebSocket";
			((string&)protocol().name) = "WebSocket";

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
		MapReader<MapParameters::Iterator> parameters(peer.properties());

		if (!peer.connected) {
			_options.clear();
			peer.onConnection(ex, _writer,parameters,_options);
		}

		if (!ex && peer.connected) {

			////////////  HTTP GET  //////////////
			if ((pPacket->command == HTTP::COMMAND_HEAD ||pPacket->command == HTTP::COMMAND_GET)) {
				// use index http option in the case of GET request on a directory
				bool methodCalled(false);
				// if no file in the path, try to invoke a method on client object
				if (pPacket->filePos == string::npos) {
					if (!_options.index.empty()) {
						if (_options.indexCanBeMethod)
							methodCalled = processMethod(ex, _options.index, parameters);

						if (!methodCalled && !ex) {
							// Redirect to the file (get name to prevent path insertion)
							string nameFile;
							filePath.appendPath('/', FileSystem::GetName(_options.index, nameFile));
						}
					} else if (!_options.indexDirectory)
						ex.set(Exception::PERMISSION, "No authorization to see the content of ", peer.path, "/");
				} else
					methodCalled = processMethod(ex, filePath.name(), parameters);

				// try to get a file if the client object had not method named like that
				if (!methodCalled && !ex) {
					parameters.reset();
					if (peer.onRead(ex, filePath, parameters, pPacket->parameters) && !ex) {
						// If onRead has been authorised, and that the file is a multimedia file, and it doesn't exists (no VOD, filePath.lastModified()==0 means "doesn't exists")
						// Subscribe for a live stream with the basename file as stream name
						if (filePath.lastModified() == 0) {
							if (pPacket->contentType == HTTP::CONTENT_ABSENT)
								pPacket->contentType = HTTP::ExtensionToMIMEType(filePath.extension(),pPacket->contentSubType);
							if ((pPacket->contentType == HTTP::CONTENT_VIDEO || pPacket->contentType == HTTP::CONTENT_AUDIO) && filePath.lastModified() == 0)
								_pListener = invoker.subscribe(ex, peer, filePath.baseName(), _writer);
						}
						if (!ex && !_pListener) {
							 // for the case of one folder displayed, search sort arguments
							UInt8 sortOptions(HTTP::SORT_ASC);
							 if (peer.properties().getString("N", invoker.buffer))
								sortOptions |= HTTP::SORT_BY_NAME;
							 else if (peer.properties().getString("M", invoker.buffer))
								sortOptions |= HTTP::SORT_BY_MODIFIED;
							 else if (peer.properties().getString("S", invoker.buffer))
								sortOptions |= HTTP::SORT_BY_SIZE;
							 if (invoker.buffer == "D")
								 sortOptions |= HTTP::SORT_DESC;
							 // HTTP get
							_writer.writeFile(filePath, sortOptions, pPacket->filePos==string::npos);
						}
					}
				}
			}
			////////////  HTTP POST  //////////////
			else if (pPacket->command == HTTP::COMMAND_POST) {
				PacketReader packetContent(pPacket->content, pPacket->contentLength);

				// Get output format from input
				_writer.contentType = pPacket->contentType;
				_writer.contentSubType = pPacket->contentSubType;

				HTTP::ReadMessageFromType(ex, *this, pPacket, packetContent);
			}
			////////////  HTTP OPTIONS  ////////////// (it is due requested when Move Redirection is sent)
			else if (pPacket->command == HTTP::COMMAND_OPTIONS) {
				processOptions(ex, pPacket);

			} else
				ex.set(Exception::PROTOCOL, "Unsupported command");


		}
	}

	if (ex)
		_writer.close(ex);
	else if(!peer.connected)
		kill(REJECTED_DEATH);
	else
		_writer.timeout.update();

}

void HTTPSession::manage() {
	if(_isWS) {
		WSSession::manage();
		return;
	}
	// timeout http session // TODO add a timeout for Listening HTTP session without media reception??
	if (peer.connected && _options.timeout > 0 && !_pListener && _writer.timeout.isElapsed(_options.timeout))
		kill(TIMEOUT_DEATH);
	else if (!_packets.empty() && _packets.front()->exception)
		_writer.close(_packets.front()->exception);
}

void HTTPSession::processOptions(Exception& ex,const shared_ptr<HTTPPacket>& pPacket) {

	// Control methods quiested
	if (pPacket->accessControlRequestMethod&HTTP::COMMAND_DELETE) {
		ex.set(Exception::PROTOCOL,"Delete not allowed");
		return;
	}

	DataWriter& response = _writer.write("200 OK", HTTP::CONTENT_ABSENT);
	BinaryWriter& writer = response.packet;

	// TODO determine the protocol (https/http)
	/*IPAddress serverAddress;
	if (!serverAddress.set(ex, peer.serverAddress))
		return;

	String::Format(invoker.buffer, "http://", serverAddress.toString());*/

	HTTP_BEGIN_HEADER(writer)
		HTTP_ADD_HEADER(writer,"Access-Control-Allow-Origin", "*")
		HTTP_ADD_HEADER(writer,"Access-Control-Allow-Methods", "GET, HEAD, PUT, PATH, POST, OPTIONS")
		HTTP_ADD_HEADER(writer,"Access-Control-Allow-Headers", "Content-Type")
	HTTP_END_HEADER(writer)
}

bool HTTPSession::processMethod(Exception& ex, const string& name, MapReader<MapParameters::Iterator>& parameters) {

	Exception exTry;
	parameters.reset();
	peer.onMessage(exTry, name, parameters);
	if (exTry && exTry.code() == Exception::SOFTWARE)
		ex.set(exTry);
	
	return !exTry;
}

} // namespace Mona
