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
#include "Mona/SOAPReader.h"
#include "Mona/SOAPWriter.h"
#include "Mona/FileSystem.h"
#include "Mona/Protocol.h"
#include "Mona/Exceptions.h"
#include <fstream>


using namespace std;


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


bool HTTPSession::buildPacket(const shared_ptr<Buffer<UInt8>>& pData, MemoryReader& packet) {

	if(_isWS)
		return WSSession::buildPacket(pData, packet);

	_request = unique_ptr<HTTPPacketReader>(new HTTPPacketReader(packet));
	HTTP::ReadHeader(*_request, _headers, _cmd, (string&)peer.path, _file, peer);

	UInt16 length = 0;
	UInt8* end = NULL;
	if (_headers.getNumber<UInt16>("content-length", length)) {
		end = packet.current() + packet.available() - 4 - length;
	} else {
		end = packet.current() + packet.available() - 4;
	}

	if(memcmp(end,"\r\n\r\n",4)!=0)
		return false;
	return true;
}


void HTTPSession::packetHandler(MemoryReader& packet) {
	if(_isWS) {
		WSSession::packetHandler(packet);
		return;
	}

	UInt32 pos = packet.position();
	UInt32 posEnd = 0;

	string oldPath;
	if(peer.connected)
		oldPath = peer.path;

	string temp = "127.0.0.1"; // TODO?
	_headers.getString("host", temp);
	Exception ex;
	((SocketAddress&)peer.serverAddress).set(ex,temp, invoker.params.HTTP.port);
	if (ex)
		WARN("serverAddress of HTTPSession ",name()," impossible to determine with the host ",temp)

	if(peer.connected && String::ICompare(oldPath,peer.path)!=0)
		peer.onDisconnection();

	// HTTP GET
	if (String::ICompare(_cmd, "GET") == 0) {

		bool connectionHasUpgrade = false;
		if (_headers.getString("connection", temp)) {
			vector<string> fields;
			for (const string& field : String::Split(temp, ",", fields, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM)) {
				if (String::ICompare(field, "upgrade") == 0) {
					connectionHasUpgrade = true;
					break;
				}
			}
		}
		
		// Ugrade to WebSocket?
		if(connectionHasUpgrade) {
			if (_headers.getString("upgrade", temp) && String::ICompare(temp, "websocket") == 0) {
				peer.onDisconnection();
				_isWS=true;
				peer.setString("protocol", "WebSocket");
				((string&)protocol().name) = "WebSocket";
				DataWriter& response = _writer.writeMessage();
				response.writeString("HTTP/1.1 101 Switching Protocols");
				response.beginObject();
				response.writeStringProperty("Upgrade","WebSocket");
				response.writeStringProperty("Connection","Upgrade");
				if (_headers.getString("sec-websocket-key", temp)) {
					WS::ComputeKey(temp);
					response.writeStringProperty("Sec-WebSocket-Accept", temp);
				}
				posEnd = packet.position();
				packet.reset(pos);
				//skip first line (HTTP/1.1 / GET ..)
				if(_request->followingType()==HTTPPacketReader::STRING)
					_request->next();

				peer.onConnection(ex, wsWriter(),*_request,response);
				if (!ex)
					response.endObject();
			} // TODO else
		} else
			processGet(ex, _file);

	} /* TODO else if (icompare(cmd,"HEAD")==0) {*/
	// WebService POST
	else if (String::ICompare(_cmd, "POST") == 0) {

		string contentType;
		if (_headers.getString("content-type", contentType) && contentType.find("text/xml") != string::npos)
			processSOAPfunction(ex, packet);
	}
	// TODO see if it is necessary (it is due requested when Move Redirection is sent)
	else if (String::ICompare(_cmd, "OPTIONS") == 0) {

		string methods;
		if (_headers.getString("access-control-request-method", methods))
			processOptions(ex, methods);
	}

	if (ex)
		sendError(ex);
	
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
	
	string filePath(peer.path);
	if (!fileName.empty())
		String::Append(filePath, "/", fileName);

	if(peer.onRead(ex, filePath)) {

		if (fileName.empty() && FileSystem::IsDirectory(filePath)) {
			// Redirect to the real path of directory
			processMove(peer.path);
			return;
		}

		auto size = FileSystem::GetSize(ex,filePath);
		if (ex)
			return;

		ifstream ifile(filePath, ios::in | ios::binary);
		if (!ifile.good()) {
			ex.set(Exception::FILE, "Impossible to open ", filePath, " file");
			return;
		}

		Time time;
		FileSystem::GetLastModified(ex, filePath, time);
		if (ex)
			return;

		Time lastTime;
		string tmp;
		if (_headers.getString("if-modified-since", tmp) && lastTime.fromString(tmp)) {

			// Not Modified => don't send the file
			if (lastTime >= time)
				processNotModified();
		}
		
		DataWriter& response = _writer.writeMessage();
		response.writeString("HTTP/1.1 200 OK");
		response.beginObject();
		response.writeStringProperty("Date", Time().toString(Time::HTTP_FORMAT, tmp));
		response.writeStringProperty("Server","Mona");

		if (_headers.getString("connection", tmp) && tmp == "keep-alive")
			response.writeStringProperty("Connection","Keep-Alive");
		else
			response.writeStringProperty("Connection","close");

		response.writeStringProperty("Last-Modified", time.toString(Time::HTTP_FORMAT, tmp));

		string extension;
		string type("text/plain");
		HTTP::MIMEType(FileSystem::GetExtension(filePath, extension), type);		
		response.writeStringProperty("Content-Type", type);
		response.writeNumberProperty("Content-Length", (double)size);
		response.endObject();

		UInt32 pos = response.stream.size();
		response.stream.next((UInt32)size);
		
		ifile.read((char*)response.stream.data() + pos, size); // TODO copy, what about peformance? multhreaded?
	}
}

void HTTPSession::processNotModified() {

	DataWriter& response = _writer.writeMessage();
	response.writeString("HTTP/1.1 304 Not Modified");
	response.beginObject();
	string stDate;
	response.writeStringProperty("Date", Time().toString(Time::HTTP_FORMAT, stDate));
	response.writeStringProperty("Server","Mona");
	response.writeStringProperty("Connection","close");
	response.writeNumberProperty("Content-Length", 0);
	response.endObject();
}

void HTTPSession::processMove(const string& filePath) {

	DataWriter& response = _writer.writeMessage();
	response.writeString("HTTP/1.1 301 Moved Permanently");
	response.beginObject();
	string stDate;
	response.writeStringProperty("Date", Time().toString(Time::HTTP_FORMAT, stDate));
	response.writeStringProperty("Server","Mona");
	response.writeStringProperty("Connection","close");

	string url;
	// TODO determine the protocol (https/http)
	String::Format(url, "http://", peer.serverAddress.host().toString(), filePath, '/');
	response.writeStringProperty("Location", url);

	string htmlMove("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">");
	htmlMove += "<html><head>";
	htmlMove += "<title>301 Moved Permanently</title>";
	htmlMove += "</head><body>";
	htmlMove += "<h1>Moved Permanently</h1>";
	htmlMove += "<p>The document has moved <a href=\"";
	htmlMove += url;
	htmlMove += "\">here</a>.</p>";
	htmlMove += "<hr>";
	htmlMove += "<address>Mona Server at ";
	String::Append(htmlMove, peer.serverAddress.host().toString(), " Port ", peer.serverAddress.port()); 
	htmlMove += "</address>";
	htmlMove += "</body></html>";

	response.writeNumberProperty("Content-Length",htmlMove.size());
	response.writeStringProperty("Content-Type","text/html; charset=utf-8");
	response.endObject();

	response.writeString(htmlMove);
}

void HTTPSession::processOptions(Exception& ex, const string& methods) {

	// Control methods quiested
	vector<string> values;
	String::Split(methods, ", ", values);
	bool ok=true;
	for (string& value : values) {
		if(String::ICompare(value, "POST")!=0 && String::ICompare(value, "GET")!=0) {

			ex.set(Exception::PROTOCOL, "Access control error : ", methods, " not allowed");
			return;
		}
	}

	DataWriter& response = _writer.writeMessage();
	string stDate;
	response.writeString("HTTP/1.1 200 OK");
	response.beginObject();
	response.writeStringProperty("Date", Time().toString(Time::HTTP_FORMAT, stDate));
	response.writeStringProperty("Server","Mona");

	string url;
	// TODO determine the protocol (https/http)
	String::Format(url, "http://", peer.serverAddress.host().toString());
	response.writeStringProperty("Access-Control-Allow-Origin", url);
					
	response.writeStringProperty("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
	response.writeStringProperty("Access-Control-Allow-Headers", "Content-Type");
	response.writeNumberProperty("Content-Length", 0);
	response.writeStringProperty("Connection","close");
	response.writeStringProperty("Content-Type", "text/xml; charset=utf-8");
	response.endObject();
}

void HTTPSession::processSOAPfunction(Exception& ex, MemoryReader& packet) {
	if(!peer.connected) {
		peer.onConnection(ex, _writer);
		if (ex)
			return;
	}
	
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
	response.writeBytes(writer.stream.data(), size);
}

void HTTPSession::sendError(Exception& ex) {

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

} // namespace Mona
