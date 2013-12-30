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
#include "Mona/HTTP/HTTPWriter.h"

#include <fstream>

using namespace std;


namespace Mona {



HTTPSender::HTTPSender(const SocketAddress& address,const shared_ptr<HTTPPacket>& pRequest) : _pRequest(pRequest),_address(address),_sizePos(0),TCPSender("TCPSender") {
	
}

void HTTPSender::writeError(int code,const string& description,bool close) {
	if (!_pRequest) {
		ERROR("No HTTP request to send that error reply")
		return;
	}
	_buffer.assign("Unknown error");
	string title;
	if (close)
		_pRequest->connection = HTTP::CONNECTION_CLOSE;
	DataWriter& response = write(String::Format(title, code, " ", HTTP::CodeToMessage(code,_buffer)));
	BinaryWriter& writer = response.writer;
	HTML_BEGIN_COMMON_RESPONSE(writer, title)
		writer.writeRaw(description.empty() ? title : description);
	HTML_END_COMMON_RESPONSE(writer, _pRequest->serverAddress)
}


bool HTTPSender::run(Exception& ex) {
	if (!_pRequest) {
		ex.set(Exception::PROTOCOL, "No HTTP request to send that reply");
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
					DataWriter& response = write("301 Moved Permanently");
					BinaryWriter& writer = response.writer;
					String::Format(_buffer, "http://", _pRequest->serverAddress, _file.path(), '/');
					HTTP_BEGIN_HEADER(response)
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
					BinaryWriter& writer = response.writer;
					HTTP_BEGIN_HEADER(response)
						HTTP_ADD_HEADER(writer,"Last-Modified", time.toString(Time::HTTP_FORMAT, _buffer))
					HTTP_END_HEADER(writer)

					HTTP::WriteDirectoryEntries(writer,_pRequest->serverAddress,_file.path(),files,_pRequest->parameters);
			
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
						BinaryWriter& writer = response.writer;
						HTTP_BEGIN_HEADER(response)
							HTTP_ADD_HEADER(writer,"Last-Modified", time.toString(Time::HTTP_FORMAT, _buffer))
						HTTP_END_HEADER(writer)


						//// Write content file and replace the "<% key %>" field by relating _pRequest->properties[key] value
						UInt32 pos = response.stream.size();
						// get file content size
						UInt32 size((UInt32)ifile.tellg());
						// reserve memory
						response.stream.next(size+_pRequest->properties.size());
						// push the entiere file content to memory
						ifile.seekg(0);
						char* current = (char*)response.stream.data() + pos;
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
									auto it = _pRequest->properties[key];
									const string& value(it==_pRequest->properties.end() ? String::Empty : it->second);
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
						response.stream.resetWriting(pos+newSize);
					}
				}
			}
		}
	
	}
	
	/// Write Content-Length if position recorded
	if (_sizePos > 0) {
		UInt32 size = _pWriter->stream.size();
		_pWriter->stream.resetWriting(_sizePos);
		
		// search \r\n\r\n
		const UInt8* end = _pWriter->stream.data()+size-4;
		const UInt8* content = _pWriter->stream.data()+_sizePos+9;
		while (content < end) {
			if (memcmp(++content, EXPAND_SIZE("\r\n\r\n")) == 0) {
				content += 4;
				break;
			}
			if (content == end)
				ERROR("HTTP header without end, unvalid packet")
		}
		_pWriter->writer.writeRaw(String::Format(_buffer,end+4-content));
		if (_pRequest->command == HTTP::COMMAND_HEAD)
			_pWriter->stream.resetWriting(content-_pWriter->stream.data());
		else
			_pWriter->stream.resetWriting(size);
	}

	/// Dump response
	Writer::DumpResponse(begin(), size(), _address);

	/// Send
	return TCPSender::run(ex);
}

DataWriter& HTTPSender::writer(const string& code, HTTP::ContentType type, const string& subType, const UInt8* data, UInt32 size) {
	if (_pWriter) {
		ERROR("HTTP response already written");
		return DataWriter::Null;
	}
	if (!_pRequest) {
		ERROR("No HTTP request to write that reply");
		return DataWriter::Null;
	}

	_pWriter.reset(type == HTTP::CONTENT_ABSENT ? new RawWriter() : HTTP::NewDataWriter(subType));

	BinaryWriter& writer = _pWriter->writer;

	Exception ex;

	// First line (HTTP/1.1 200 OK)
	UInt16 value(200);
	writer.writeRaw("HTTP/1.1 ");
	writer.writeRaw(code);
	if (String::ToNumber<UInt16>(code, value)) {
		writer.writeRaw(" ");
		_buffer.assign("Unknown");
		writer.writeRaw(HTTP::CodeToMessage(value, _buffer));
	}

	// Date + Mona
	writer.writeRaw("\r\nDate: "); writer.writeRaw(Time().toString(Time::HTTP_FORMAT, _buffer));
	writer.writeRaw("\r\nServer: Mona");

	// Connection type, same than request!
	UInt8 connection = _pRequest->connection;
	if (connection&HTTP::CONNECTION_KEEPALIVE) {
		writer.writeRaw("\r\nConnection: keep-alive");
		if (connection&HTTP::CONNECTION_UPGRADE)
			writer.writeRaw(", upgrade");
		if (connection&HTTP::CONNECTION_CLOSE)
			writer.writeRaw(", close");
	} else if (connection&HTTP::CONNECTION_UPGRADE) {
		writer.writeRaw("\r\nConnection: upgrade");
		if (connection&HTTP::CONNECTION_CLOSE)
			writer.writeRaw(", close");
	} else if (connection&HTTP::CONNECTION_CLOSE)
		writer.writeRaw("\r\nConnection: close");

	// Content Type
	if (type != HTTP::CONTENT_ABSENT) {
		writer.writeRaw("\r\nContent-Type: ");
		writer.writeRaw(HTTP::FormatContentType(type, subType, _buffer));
		
		// Content Length
		if (data) {
			writer.writeRaw("\r\nContent-Length: ");
			writer.writeRaw(String::Format(_buffer, size));
		} else if(size==0) { // if size!=0 means that we want not writing a content-length
			// reserve place to add length on sending
			writer.writeRaw("\r\nContent-Length:           ");
			_sizePos = _pWriter->stream.size()-10;
		} else {
			// here it means that we are on a live streaming, without size limit, so we have to signal the cache-control
			//writer.writeRaw("\r\nContent-Length: 9999999999");
			writer.writeRaw("\r\nCache-Control: no-cache, no-store\r\nPragma: no-cache");
		}
	}

	writer.writeRaw("\r\n\r\n");
	if (data && size > 0)
		writer.writeRaw(data, size);
	return !data || size>0 ? *_pWriter : DataWriter::Null;
}

BinaryWriter& HTTPSender::writeRaw() {
	if (_pWriter) {
		ERROR("HTTP response already written");
		return DataWriter::Null.writer;
	}
	_pWriter.reset(new RawWriter());
	return _pWriter->writer;
}


} // namespace Mona
