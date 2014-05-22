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

#include "Mona/HTTP/HTTPSender.h"
#include "Mona/FileSystem.h"
#include "Mona/Files.h"
#include "Mona/RawWriter.h"
#include "Mona/Logs.h"
#include "Mona/HTTP/HTTPWriter.h"

#include <fstream>

using namespace std;


namespace Mona {



HTTPSender::HTTPSender(const SocketAddress& address, HTTPPacket& request,const PoolBuffers& poolBuffers) :
	_serverAddress(request.serverAddress),
	_ifModifiedSince(request.ifModifiedSince),
	_connection(request.connection),
	_command(request.command),
	_pInfos(request.pullSendingInfos()),
	_address(address),
	_sizePos(0),
	_sortOptions(0),
	_isApp(false),
	_poolBuffers(poolBuffers),
	TCPSender("TCPSender") {

}

void HTTPSender::writeError(int code,const string& description,bool close) {
	_buffer.assign("Unknown error");
	string title;
	if (close)
		_connection = HTTP::CONNECTION_CLOSE;
	DataWriter& response = write(String::Format(title, code, " ", HTTP::CodeToMessage(code,_buffer)));
	BinaryWriter& writer = response.packet;
	HTML_BEGIN_COMMON_RESPONSE(writer, title)
		writer.writeRaw(description.empty() ? title : description);
	HTML_END_COMMON_RESPONSE(writer, _serverAddress)
}


bool HTTPSender::run(Exception& ex) {

	if (!_pWriter) {

		//// GET FILE
		Date date;
		// Not Modified => don't send the file
		if (_file.lastModified()>0 && _ifModifiedSince >= _file.lastModified()) {
			write("304 Not Modified", HTTP::CONTENT_ABSENT);
		} else {
			// file doesn't exist
			if (_file.lastModified()==0)
				writeError(404, String::Format(_buffer,"File ", _file.path(), " doesn't exist"));
			else {
				Exception exIgnore;
				Files files(exIgnore, _file.fullPath());
				// Folder
				if (!exIgnore) {
					// Connected to parent => redirect to url + '/'
					if (!_isApp) {
						// Redirect to the real path of directory
						DataWriter& response = write("301 Moved Permanently");
						BinaryWriter& writer = response.packet;
						String::Format(_buffer, "http://", _serverAddress, _file.path(), '/');
						HTTP_BEGIN_HEADER(writer)
							HTTP_ADD_HEADER(writer, "Location", _buffer)
						HTTP_END_HEADER(writer)
						HTML_BEGIN_COMMON_RESPONSE(writer, "Moved Permanently")
							writer.writeRaw("The document has moved <a href=\"", _buffer, "\">here</a>.");
						HTML_END_COMMON_RESPONSE(writer, _buffer)
					} else {
						DataWriter& response = write("200 OK");
						BinaryWriter& writer = response.packet;
						HTTP_BEGIN_HEADER(writer)
							HTTP_ADD_HEADER(writer,"Last-Modified", date.toString(Date::HTTP_FORMAT, _buffer))
						HTTP_END_HEADER(writer)

						HTTP::WriteDirectoryEntries(writer,_serverAddress,_file.path(),files,_sortOptions);
					}
				} 
				// File
				else {
					ifstream ifile(_file.fullPath(), ios::in | ios::binary | ios::ate);
					if (!ifile.good()) {
						exIgnore.set(Exception::NIL, "Impossible to open ", _file.path(), " file");
						writeError(423,  exIgnore.error());
					} else {
						// determine the content-type
						string subType;
						HTTP::ContentType type = HTTP::ExtensionToMIMEType(_file.extension(), subType);	

						DataWriter& response = write("200 OK", type,subType);
						PacketWriter& packet = response.packet;
						HTTP_BEGIN_HEADER(packet)
							HTTP_ADD_HEADER(packet,"Last-Modified", date.toString(Date::HTTP_FORMAT, _buffer))
						HTTP_END_HEADER(packet)

						// TODO see if filter is correct
						if (type == HTTP::CONTENT_TEXT && _pInfos && _pInfos->parameters.count())
							replaceTemplateTags(packet, ifile, _pInfos->parameters, _pInfos->sizeParameters);
						else {

							// push the entire file content to memory
							UInt32 size = (UInt32)ifile.tellg();
							ifile.seekg(0);
							char* current = (char*)packet.buffer(size); // reserve memory for file
							ifile.read(current, size);
						}
					}
				}
			}
		}
	
	}
	
	/// Write Content-Length if position recorded
	if (_sizePos > 0) {
		PacketWriter& packet(_pWriter->packet);
		UInt32 size = packet.size();

		// search \r\n\r\n of header part
		const UInt8* end = packet.data()+size-4;
		const UInt8* content = packet.data()+_sizePos+9;
		while (content < end) {
			if (memcmp(++content, EXPAND_SIZE("\r\n\r\n")) == 0) {
				content += 4;
				break;
			}
			if (content == end)
				ERROR("HTTP header without end, unvalid packet")
		}

		// write content-length
		String::Format(_buffer, end + 4 - content);
		memcpy((UInt8*)packet.data()+_sizePos,_buffer.c_str(),_buffer.size());

		if (_command == HTTP::COMMAND_HEAD)
			packet.clear(content-packet.data());
		else
			packet.clear(size);
	}

	/// Dump response
	Session::DumpResponse(data(), size(), _address);

	/// Send
	return TCPSender::run(ex);
}

DataWriter& HTTPSender::writer(const string& code, HTTP::ContentType type, const string& subType, const UInt8* data, UInt32 size) {
	if (_pWriter) {
		ERROR("HTTP response already written");
		return DataWriter::Null;
	}

	_pWriter.reset(type == HTTP::CONTENT_ABSENT ? new RawWriter(_poolBuffers) : HTTP::NewDataWriter(_poolBuffers,subType));

	PacketWriter& packet = _pWriter->packet;

	Exception ex;

	// First line (HTTP/1.1 200 OK)
	UInt16 value(200);
	packet.writeRaw("HTTP/1.1 ");
	packet.writeRaw(code);
	if (String::ToNumber<UInt16>(code, value)) {
		packet.writeRaw(" ");
		_buffer.assign("Unknown");
		packet.writeRaw(HTTP::CodeToMessage(value, _buffer));
	}

	// Date + Mona
	packet.writeRaw("\r\nDate: "); packet.writeRaw(Date().toString(Date::HTTP_FORMAT, _buffer));
	packet.writeRaw("\r\nServer: Mona");

	// Connection type, same than request!
	if (_connection&HTTP::CONNECTION_KEEPALIVE) {
		packet.writeRaw("\r\nConnection: keep-alive");
		if (_connection&HTTP::CONNECTION_UPGRADE)
			packet.writeRaw(", upgrade");
		if (_connection&HTTP::CONNECTION_CLOSE)
			packet.writeRaw(", close");
	} else if (_connection&HTTP::CONNECTION_UPGRADE) {
		packet.writeRaw("\r\nConnection: upgrade");
		if (_connection&HTTP::CONNECTION_CLOSE)
			packet.writeRaw(", close");
	} else if (_connection&HTTP::CONNECTION_CLOSE)
		packet.writeRaw("\r\nConnection: close");

	// Set Cookies
	if (_pInfos) {
		for(const string& cookie : _pInfos->setCookies)
			packet.writeRaw("\r\nSet-Cookie: ", cookie);
	}

	// Content Type
	if (type != HTTP::CONTENT_ABSENT) {
		packet.writeRaw("\r\nContent-Type: ");
		packet.writeRaw(HTTP::FormatContentType(type, subType, _buffer));
		
		// Content Length
		if (data) {
			packet.writeRaw("\r\nContent-Length: ");
			packet.writeRaw(String::Format(_buffer, size));
		} else if(size==0) { // if size!=0 means that we want not writing a content-length
			// reserve place to add length on sending
			packet.writeRaw("\r\nContent-Length:           ");
			_sizePos = packet.size()-10;
		} else {
			// here it means that we are on a live streaming, without size limit, so we have to signal the cache-control
			//writer.writeRaw("\r\nContent-Length: 9999999999");
			packet.writeRaw("\r\nCache-Control: no-cache, no-store\r\nPragma: no-cache");
		}
	}

	packet.writeRaw("\r\n\r\n");
	if (data && size > 0)
		packet.writeRaw(data, size);
	return !data || size>0 ? *_pWriter : DataWriter::Null;
}

BinaryWriter& HTTPSender::writeRaw(const PoolBuffers& poolBuffers) {
	if (_pWriter) {
		ERROR("HTTP response already written");
		return DataWriter::Null.packet;
	}
	_pWriter.reset(new RawWriter(poolBuffers));
	return _pWriter->packet;
}

void HTTPSender::replaceTemplateTags(PacketWriter& packet, ifstream& ifile, const Parameters& parameters, UInt32 sizeParameters) {

	UInt32 pos = packet.size();
	// get file content size
	UInt32 size = (UInt32)ifile.tellg();

	// push the entire file content to memory
	ifile.seekg(0);
	char* current = (char*)packet.buffer(size+sizeParameters); // reserve more memory to change <%name%> field
	ifile.read(current, size);
	// iterate on content to replace "<% key %>" fields
	UInt32 newSize(size);
	const char* end = current + size;
	UInt8 step(0);
	char* signifiant(NULL);
	UInt32 keyLength(0);
	const char* keyBegin(NULL);
	string key;
	while (current < end) {
		char c = *current;
		if (step < 2) {
			if (step==0 && c == '<')
				++step;
			else if (step == 1 && c == '%') {
				++step;
				signifiant = current-1;
				keyBegin = NULL;
				keyLength = 0;
				key.clear();
			} else
				step = 0;
		} else {
			// in <% ... %>
			if (step==2 && c == '%')
				++step;
			else if (step==3 && c == '>')
				++step;
			else {
				step = 2;
				// search key
				if (key.empty()) {
					if (!isspace(c)) {
						if (!keyBegin)
							keyBegin = current;
						++keyLength;
					} else if(keyBegin)
						key.assign(keyBegin, keyLength);
				}
			}

			if (step == 4) {
				step = 0;

				if (keyBegin)
					key.assign(keyBegin, keyLength);
				UInt32 available(current+1-signifiant);
				string value;
				parameters.getString(key, value);
				// give the size available required
				if (available < value.size()) {
					available = value.size()-available; // to add
					newSize += available;
					memmove(current+1,current+1+available,end-current-1);
					current += available-1;
				} else if (available>value.size()) {
					available = available-value.size(); // to remove
					newSize -= available;
					memcpy(current+1-available,current+1,end-current-1);
					current -= available-1;
				}
				// replace <% key %> by value
				if (!value.empty())
					memcpy(signifiant,value.c_str(),value.size());
			}
		}
		++current;
	}

	// resize final stream
	packet.clear(pos+newSize);
}

} // namespace Mona
