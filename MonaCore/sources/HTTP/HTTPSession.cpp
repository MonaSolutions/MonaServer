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
#include "Mona/MapReader.h"
#include "Mona/SOAPWriter.h"
#include "Mona/Protocol.h"
#include "Mona/Exceptions.h"
#include "Mona/FileSystem.h"
#include "Mona/HTTP/HTTPPacketBuilding.h"


using namespace std;


namespace Mona {


HTTPSession::HTTPSession(const SocketAddress& address, Protocol& protocol, Invoker& invoker) : WSSession(address, protocol, invoker), _isWS(false), _writer(*this),_ppBuffer(new PoolBuffer(invoker.poolBuffers)), _pListener(NULL) {

}


HTTPSession::~HTTPSession() {
	kill();
}

void HTTPSession::kill(){
	if(died)
		return;
	if (_isWS)
		wsWriter().close(WS::CODE_ENDPOINT_GOING_AWAY);
	if (_pListener) {
		invoker.unsubscribe(peer, _pListener->publication.name());
		_pListener = NULL;
	}
	WSSession::kill();
}

bool HTTPSession::buildPacket(PacketReader& packet) {
	if(_isWS)
		return WSSession::buildPacket(packet);
	// consumes all!
	shared_ptr<HTTPPacketBuilding> pHTTPPacketBuilding(new HTTPPacketBuilding(invoker,rawBuffer(), _ppBuffer));
	_packets.emplace_back(pHTTPPacketBuilding->pPacket);
	decode<HTTPPacketBuilding>(pHTTPPacketBuilding);
	return true;
}

const shared_ptr<HTTPPacket>& HTTPSession::packet() {
	while (!_packets.empty() && _packets.front().unique())
		_packets.pop_front();
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

	string oldPath;
	if(peer.connected)
		oldPath = peer.path;

	// HTTP is a simplex communication, so if request, remove possible old subscription
	if (_pListener) {
		invoker.unsubscribe(peer, _pListener->publication.name());
		_pListener = NULL;
	}

	////  fill peers infos
	peer.setPath(pPacket->path);
	peer.setQuery(pPacket->query);
	peer.setServerAddress(pPacket->serverAddress);
	peer.properties().setNumber("HTTPVersion", pPacket->version); // TODO check how is named for AMF
	Util::UnpackQuery(peer.query,peer.properties());
	FilePath filePath(peer.path);
	if (pPacket->filePos != string::npos)
		((string&)peer.path).erase(pPacket->filePos - 1);


	//// Disconnection if path has changed
	if(peer.connected && String::ICompare(oldPath,peer.path)!=0)
		peer.onDisconnection();

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
						if (_options.indexCanBeMethod) {
							Exception exTry;
							parameters.reset();
							peer.onMessage(exTry, _options.index,parameters,HTTPWriter::RAW);
							if (exTry) {
								if (exTry.code() == Exception::SOFTWARE)
									ex.set(exTry);
							} else
								methodCalled = true;
						}
						if (!methodCalled && !ex) {
							filePath.path(filePath.path(),"/",_options.index);
						}
					} else if (!_options.indexDirectory)
						ex.set(Exception::PERMISSION, "No authorization to see the content of ", peer.path,"/");
				}

				// try to get a file if the client object had not method named like that
				if (!methodCalled && !ex) {
					parameters.reset();
					if (peer.onRead(ex, filePath, parameters, pPacket->parameters) && !ex) {
						// If onRead has been authorised, and that the file is a multimedia file, and it doesn't exists (no VOD, filePath.lastModified()==0 means "doesn't exists")
						// Subscribe for a live stream with the basename file as stream name
						if (filePath.lastModified() == 0) {
							if (pPacket->contentType == HTTP::CONTENT_ABSENT)
								pPacket->contentType = HTTP::ExtensionToMIMEType(filePath.extension(),pPacket->contentSubType);
							if ((pPacket->contentType == HTTP::CONTENT_VIDEO || pPacket->contentType == HTTP::CONTENT_AUDIO) && filePath.lastModified() == 0) {
								
								if(pPacket->contentSubType == "x-flv")
									_writer.mediaType = MediaContainer::FLV;
								else if(pPacket->contentSubType == "mpeg")
									_writer.mediaType = MediaContainer::MPEG_TS;
								else
									ex.set(Exception::APPLICATION, "HTTP streaming for a ",pPacket->contentSubType," unsupported");
								
								if (!ex) {
									_pListener = invoker.subscribe(ex, peer, filePath.baseName(), _writer);
									// write a HTTP header without content-length (data==NULL and size>0) + HEADER
									MediaContainer::Write(_writer.mediaType,_writer.write("200", pPacket->contentType, pPacket->contentSubType, NULL, 1).packet);
								}
								
							}
								
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
							_writer.writeFile(filePath,sortOptions);
						}
					}
				}
			}
			////////////  HTTP POST  //////////////
			else if (pPacket->command == HTTP::COMMAND_POST) {
				// TODO publication!
				string contentType;
				if (pPacket->contentType == HTTP::CONTENT_TEXT && pPacket->contentSubType == "xml")
					processSOAPfunction(ex, reader);
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
		kill();
	else
		_writer.timeout.update();

	// erase request, we are in a pull mode (response just if request before)
	if (_writer.pRequest) {
		// erase previous parameters
		peer.properties().clear();
		// erase previous request
		_writer.pRequest.reset();
	}
}

void HTTPSession::manage() {
	if(_isWS) {
		WSSession::manage();
		return;
	}
	// timeout http session
	if (peer.connected && _options.timeout > 0 && !_pListener && _writer.timeout.isElapsed(_options.timeout))
		kill();
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
	String::Format(invoker.buffer, "http://", peer.serverAddress);

	HTTP_BEGIN_HEADER(writer)
		HTTP_ADD_HEADER(writer,"Access-Control-Allow-Origin", invoker.buffer)
		HTTP_ADD_HEADER(writer,"Access-Control-Allow-Methods", "GET, HEAD, PUT, PATH, POST, OPTIONS")
		HTTP_ADD_HEADER(writer,"Access-Control-Allow-Headers", "Content-Type")
	HTTP_END_HEADER(writer)
}

void HTTPSession::processSOAPfunction(Exception& ex, PacketReader& packet) {
	/*
	// Get function name
	SOAPReader reader(packet);
	string function = "onMessage";
	bool external = false;
	if(reader.followingType()==SOAPReader::OBJECT) {
		reader.readObject(function, external);

		string tmp;
		while(reader.readItem(tmp)!=SOAPReader::END)
			reader.readString(tmp);

		// extract function from namespace
		vector<string> nsAndFunction;
		String::Split(function, ":", nsAndFunction);
		if (nsAndFunction.size() > 1)
			function = nsAndFunction[1];
	}

	// Try to call lua function
	SOAPWriter writer;
	peer.onMessage(ex, function, reader, writer);
	if (ex)
		return;

	writer.end();
	UInt32 size = writer.stream.size();

	// Send HTTP Header
	DataWriter& response = _writer.writeMessage();
	response.writeString("HTTP/1.1 200 OK");
	response.beginObject();
	string stDate;
	response.writeStringProperty("Date", Time().toString(Time::HTTP_FORMAT, stDate));
	response.writeStringProperty("Server","Mona");

	string url;
	// TODO determine the protocol (https/http)
	String::Format(url, "http://", peer.serverAddress.host().toString());
	response.writeStringProperty("Access-Control-Allow-Origin", url);

	response.writeStringProperty("Content-Type", "text/xml; charset=utf-8");
	response.writeNumberProperty("Content-Length", (double)size);
	response.writeStringProperty("Connection","close");
	response.endObject();

	// Send SOAP Response
	response.writeBytes(writer.stream.data(), size);*/
}


} // namespace Mona
