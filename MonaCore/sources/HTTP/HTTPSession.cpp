/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
#include "Mona/HTTPPacketReader.h"
#include "Mona/Exceptions.h"
#include "Poco/String.h"
#include "Poco/Format.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/FileStream.h"
#include "Poco/StringTokenizer.h"
#include <cstring> // for memcmp


using namespace std;
using namespace Poco;

namespace Mona {


HTTPSession::HTTPSession(const SocketAddress& address, Protocol& protocol, Invoker& invoker) : WSSession(address, protocol, invoker), _isWS(false), _writer(*this) {

}


HTTPSession::~HTTPSession() {
	if(!died) {
		if(_isWS)
			wsWriter().close(WS_ENDPOINT_GOING_AWAY);
		// TODO else for HTTP?
	}
}

bool HTTPSession::buildPacket(MemoryReader& data,UInt32& packetSize) {
	if(_isWS)
		return WSSession::buildPacket(data,packetSize);
	const UInt8* end = data.current()+data.available()-4;
	if(memcmp(end,"\r\n\r\n",4)!=0)
		return false;
	packetSize = data.available();
	return true;
}


void HTTPSession::packetHandler(MemoryReader& packet) {
	if(_isWS) {
		WSSession::packetHandler(packet);
		return;
	}

	UInt32 pos = packet.position();
	UInt32 posEnd = 0;

	string cmd,file,oldPath;
	if(peer.connected)
			oldPath = peer.path;
	HTTPPacketReader request(packet);

	MapParameters headers;
	HTTP::ReadHeader(request, headers, cmd, (string&)peer.path, file, peer);
	string temp = "127.0.0.1"; // TODO?
	headers.getString("host", temp);
	Exception ex;
	((SocketAddress&)peer.serverAddress).set(ex,temp, invoker.params.HTTP.port);
	if (ex)
		WARN("serverAddress of HTTPSession ",id," impossible to determinate with the host ",temp)

	if(peer.connected && icompare(oldPath,peer.path)!=0)
		peer.onDisconnection();

	if (icompare(cmd, "GET") == 0) {
		bool connectionHasUpgrade = false;
		if (headers.getString("connection", temp)) {
			for (const string& field : StringTokenizer(temp, ",", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM)) {
				if (icompare(field, "upgrade") == 0) {
					connectionHasUpgrade = true;
					break;
				}
			}
		}
				
		if(connectionHasUpgrade) {
			if (headers.getString("upgrade", temp) && icompare(temp, "websocket") == 0) {
				peer.onDisconnection();
				_isWS=true;
				((string&)peer.protocol) = "WebSocket";
				((string&)protocol.name) = "WebSocket";
				DataWriter& response = _writer.writeMessage();
				response.writeString("HTTP/1.1 101 Switching Protocols");
				response.beginObject();
				response.writeStringProperty("Upgrade","WebSocket");
				response.writeStringProperty("Connection","Upgrade");
				if (headers.getString("sec-websocket-key", temp)) {
					WS::ComputeKey(temp);
					response.writeStringProperty("Sec-WebSocket-Accept", temp);
				}
				posEnd = packet.position();
				packet.reset(pos);
				//skip first line (HTTP/1.1 / GET ..)
				if(request.followingType()==HTTPPacketReader::STRING)
					request.next();

				peer.onConnection(ex, wsWriter(),request,response);
				if (!ex)
					response.endObject();
			} // TODO else
		} else
			processGet(ex, file);

	} /* TODOelse if (icompare(cmd,"HEAD")==0) {
			response.writeString("HTTP/1.1 200 OK");
		response.beginObject();
		string stDate;
		Time().toString(stDate, Time::HTTP_FORMAT);
		response.writeStringProperty("Date", stDate);
		response.writeStringProperty("Server","Mona");
		if(!peer.connected)
			peer.onConnection(_writer,response);
		if(file.empty())
			file = "index.html";
		if(peer.onRead(file)) {
			Path path(file);
				
			FileInputStream		fileStream(file);
			fileStream.seekg(0,ios::end);
			FileInputStream::pos_type size = fileStream.tellg();
			fileStream.seekg(0, std::ios::beg);

			string type;
			HTTP::MIMEType(path.getExtension(),type);
			response.writeStringProperty("Content-Type",type);
			response.writeNumberProperty("Content-Length",size);
			//response.writeStringProperty("Connection","close"); // TODO support keep-alive?
		}// TODO else??
		response.endObject();
	} // TODO else */

	if (ex) {
		
		string firstLine,title;
		switch(ex.code()) {
			case Exception::FILE:
				firstLine.assign("HTTP/1.1 404 Not Found");
				break;
			case Exception::APPLICATION:
				firstLine.assign("HTTP/1.1 500 Internal Server Error");
				break;
			default:
				firstLine.assign("HTTP/1.1 520 ");
				HTTP::CodeToMessage(520,title);
		}
		DataWriter& response = _writer.writeMessage();
		response.writeString(firstLine);
		response.beginObject();
		string stDate;
		response.writeStringProperty("Date", Time().toString(Time::HTTP_FORMAT, stDate));
		response.writeStringProperty("Server","Mona");
		response.writeStringProperty("Connection","Close");
		response.endObject();
		string error("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>");
		error += title;
		error += "</title></head><body><h1>";
		error += title;
		error += "</h1><p>";
		error += ex.error();
		error += "</p><hr><address>Mona Server at ";
		error += peer.serverAddress.toString();
		error += "</address></body></html>\r\n";
		response.writeString(error);
		_writer.close();
		kill();
	}
	
	
	if(!peer.connected) {
		kill();
		return;
	}

	_writer.flush();

	// continue the packet (because here the packet can contains multiple sub packet, cause the buildPacket implementation)
	if(posEnd>0)
		packet.reset(posEnd);
	if(packet.available())
		packetHandler(packet);
}

void HTTPSession::manage() {
	if(_isWS) {
		WSSession::manage();
		return;
	}
	/* TODO ping?
	if(peer.connected && _time.elapsed()>60000000) {
		_writer.writePing();
		_time.update();
	}*/
	_writer.flush();
}


void HTTPSession::processGet(Exception& ex, const string& fileName) {
	if(!peer.connected) {
		peer.onConnection(ex, _writer);
		if (ex)
			return;
	}

	DataWriter& response = _writer.writeMessage();
	response.writeString("HTTP/1.1 200 OK");
	response.beginObject();
	string stDate;
	response.writeStringProperty("Date", Time().toString(Time::HTTP_FORMAT, stDate));
	response.writeStringProperty("Server","Mona");
	//response.writeStringProperty("Connection","close"); // TODO support keep-alive?
	
	
	string filePath(peer.path);
	filePath.append("/");
	if (fileName.empty())
		filePath.append("index.html");
	else
		filePath.append(fileName);

	if(peer.onRead(filePath)) {
		File file(filePath);
		if (!file.exists()) {
			response.clear();
			ex.set(Exception::FILE,"File ", filePath, " doesn't exist");
			return;
		}
		auto size = file.getSize();

		Path path(filePath);
		
		string type("text/plain");
		HTTP::MIMEType(path.getExtension(), type);
		response.writeStringProperty("Content-Type", type);
		response.writeNumberProperty("Content-Length", (double)size);
		response.endObject();

		UInt32 pos = response.stream.size();
		response.stream.next((UInt32)size);
		FileInputStream(filePath).read((char*)response.stream.data() + pos, size); // TODO copy, what about peformance? multhreaded?
	} else
		response.clear();// TODO else??
}


} // namespace Mona
