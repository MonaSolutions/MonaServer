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



HTTPSender::HTTPSender(const SocketAddress& address,const shared_ptr<HTTPPacket>& pRequest) : _pRequest(pRequest),_address(address),_sizePos(0),TCPSender("TCPSender"),_sortOptions(0) {
	
}

void HTTPSender::writeError(int code,const string& description,bool close) {
	if (!_pRequest) {
		ERROR("No HTTP request to send this error reply")
		return;
	}
	_buffer.assign("Unknown error");
	string title;
	if (close)
		_pRequest->connection = HTTP::CONNECTION_CLOSE;
	DataWriter& response = write(String::Format(title, code, " ", HTTP::CodeToMessage(code,_buffer)));
	BinaryWriter& writer = response.packet;
	HTML_BEGIN_COMMON_RESPONSE(writer, title)
		writer.writeRaw(description.empty() ? title : description);
	HTML_END_COMMON_RESPONSE(writer, _pRequest->serverAddress)
}


bool HTTPSender::run(Exception& ex) {
	if (!_pRequest && (!_pWriter || _sizePos>0)) { // accept just HTTPSender::writeRaw call, for media streaming
		ex.set(Exception::PROTOCOL, "No HTTP request to send the reply");
		return false;
	}

	if (!_pWriter) {

		//// GET FILE
		Time time;
		// Not Modified => don't send the file
		if (_file.lastModified()>0 && _pRequest->ifModifiedSince >= _file.lastModified()) {
			write("304 Not Modified", HTTP::CONTENT_ABSENT);
		} else {
			if (_file.lastModified()==0) {
				// file doesn't exist, test directory
				string dir(_file.fullPath());
				if (FileSystem::Exists(FileSystem::MakeDirectory(dir))) {
					// Redirect to the real path of directory
					DataWriter& response = write("301 Moved Permanently"); // TODO check that it happens sometimes or never!
					BinaryWriter& writer = response.packet;
					String::Format(_buffer, "http://", _pRequest->serverAddress, _file.path(), '/');
					HTTP_BEGIN_HEADER(writer)
						HTTP_ADD_HEADER(writer, "Location", _buffer)
					HTTP_END_HEADER(writer)
					HTML_BEGIN_COMMON_RESPONSE(writer, "Moved Permanently")
						writer.writeRaw("The document has moved <a href=\"", _buffer, "\">here</a>.");
					HTML_END_COMMON_RESPONSE(writer, _buffer)
				} else
					writeError(404, String::Format(_buffer,"File ", _file.path(), " doesn't exist"));
			} else {
				Exception exIgnore;
				Files files(exIgnore, _file.fullPath());
				if (!exIgnore) {
					// Folder

					DataWriter& response = write("200 OK");
					BinaryWriter& writer = response.packet;
					HTTP_BEGIN_HEADER(writer)
						HTTP_ADD_HEADER(writer,"Last-Modified", time.toString(Time::HTTP_FORMAT, _buffer))
					HTTP_END_HEADER(writer)

					HTTP::WriteDirectoryEntries(writer,_pRequest->serverAddress,_file.path(),files,_sortOptions);
			
				} else {
					// File
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
							HTTP_ADD_HEADER(packet,"Last-Modified", time.toString(Time::HTTP_FORMAT, _buffer))
						HTTP_END_HEADER(packet)

						// TODO see if filter is correct
						if (type == HTTP::CONTENT_TEXT)
							ReplaceTemplateTags(packet, ifile, _pRequest->parameters);
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

		if (_pRequest->command == HTTP::COMMAND_HEAD)
			packet.clear(content-packet.data());
		else
			packet.clear(size);
	}

	/// Dump response
	Writer::DumpResponse(data(), size(), _address);

	/// Send
	return TCPSender::run(ex);
}

DataWriter& HTTPSender::writer(const string& code, HTTP::ContentType type, const string& subType, const UInt8* data, UInt32 size) {
	if (_pWriter) {
		ERROR("HTTP response already written");
		return DataWriter::Null;
	}
	if (!_pRequest) {
		ERROR("No HTTP request to write the reply");
		return DataWriter::Null;
	}

	_pWriter.reset(type == HTTP::CONTENT_ABSENT ? new RawWriter(_pRequest->poolBuffers()) : HTTP::NewDataWriter(_pRequest->poolBuffers(),subType));

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
	packet.writeRaw("\r\nDate: "); packet.writeRaw(Time().toString(Time::HTTP_FORMAT, _buffer));
	packet.writeRaw("\r\nServer: Mona");

	// Connection type, same than request!
	UInt8 connection = _pRequest->connection;
	if (connection&HTTP::CONNECTION_KEEPALIVE) {
		packet.writeRaw("\r\nConnection: keep-alive");
		if (connection&HTTP::CONNECTION_UPGRADE)
			packet.writeRaw(", upgrade");
		if (connection&HTTP::CONNECTION_CLOSE)
			packet.writeRaw(", close");
	} else if (connection&HTTP::CONNECTION_UPGRADE) {
		packet.writeRaw("\r\nConnection: upgrade");
		if (connection&HTTP::CONNECTION_CLOSE)
			packet.writeRaw(", close");
	} else if (connection&HTTP::CONNECTION_CLOSE)
		packet.writeRaw("\r\nConnection: close");

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

void HTTPSender::ReplaceTemplateTags(PacketWriter& packet, ifstream& ifile, MapWriter<std::map<std::string,std::string>>& parameters) {

	UInt32 pos = packet.size();
	// get file content size
	UInt32 size = (UInt32)ifile.tellg();

	// push the entire file content to memory
	ifile.seekg(0);
	char* current = (char*)packet.buffer(size+parameters.size()); // reserve more memory to change <%name%> field
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
				auto& it = parameters[key];
				const string& value(it==parameters.end() ? String::Empty : it->second);
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
