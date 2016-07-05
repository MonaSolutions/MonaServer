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
#include "Mona/StringWriter.h"
#include "Mona/MIME.h"
#include "Mona/Logs.h"
#include "Mona/HTTP/HTTPWriter.h"

#include <fstream>

using namespace std;


namespace Mona {



HTTPSender::HTTPSender(const SocketAddress& address, HTTPPacket& request,const PoolBuffers& poolBuffers, const string& relativePath,PoolBuffer& pSetCookieBuffer) :
	_serverAddress(request.serverAddress),
	_origin(request.origin),
	_ifModifiedSince(request.ifModifiedSince),
	_connection(request.connection),
	_command(request.command),
	_address(address),
	_sizePos(0),
	_poolBuffers(poolBuffers),
	_appPath(relativePath),
	_newHeaders(false),
	_request(request),
	_pSetCookieBuffer(poolBuffers),
	TCPSender("TCPSender") {
	_pSetCookieBuffer.swap(pSetCookieBuffer);
}

void HTTPSender::onSent(Socket& socket) {
	if (_connection != HTTP::CONNECTION_CLOSE)
		return;
	// disconnect socket if _connection==HTTP::CONNECTION_CLOSE
	Exception ex;
	socket.shutdown(ex);
}


bool HTTPSender::run(Exception& ex) {

	if (!_pWriter) {

		//// GET FILE
		Date date;

		if (!_file.exists()) {
			if (_file.isFolder())
				writeError(404, "The requested URL ", _appPath, '/',_file.name(), " was not found on this server");
			else {
				// last "/" is missing? is it a folder instead of file?
				if (_file.makeFolder().exists()) {
					// Redirect to the real folder path
					BinaryWriter& writer(write("301 Moved Permanently").packet);
					String::Format(_buffer, "http://", _serverAddress,_appPath, '/',_file.name(), '/');
					HTTP_BEGIN_HEADER(writer)
						HTTP_ADD_HEADER("Location", _buffer)
						HTTP_END_HEADER
						HTML_BEGIN_COMMON_RESPONSE(writer, EXPAND("Moved Permanently"))
						writer.write(EXPAND("The document has moved <a href=\"")).write(_buffer).write(EXPAND("\">here</a>."));
					HTML_END_COMMON_RESPONSE(_buffer)
				} else
					writeError(404, "The requested URL ",_appPath, '/',_file.name() , " was not found on this server");
			}
		} else if (!_pFileParams->count() && _ifModifiedSince && _ifModifiedSince >= _file.lastModified()) {
			// not modified if there is no parameters file (impossible to determinate if the parameters have changed since the last request)
			write("304 Not Modified", HTTP::CONTENT_ABSENT);
		} else if (_file.isFolder()) {
			/// Folder
			BinaryWriter& writer(write("200 OK", HTTP::CONTENT_TEXT, "html; charset=utf-8").packet);
			HTTP_BEGIN_HEADER(writer)
				HTTP_ADD_HEADER("Last-Modified", date.toString(Date::HTTP_FORMAT, _buffer))
			HTTP_END_HEADER

			HTTP::SortOrder		sortOrder(HTTP::SORT_ASC);
			HTTP::SortField		sortField(HTTP::SORT_BY_NAME);

			if (_pFileParams) {
				if (_pFileParams->getString("N", _buffer))
					sortField = HTTP::SORT_BY_NAME;
				else if (_pFileParams->getString("M", _buffer))
					sortField = HTTP::SORT_BY_MODIFIED;
				else if (_pFileParams->getString("S", _buffer))
					sortField = HTTP::SORT_BY_SIZE;
				if (_buffer == "D")
					sortOrder = HTTP::SORT_DESC;
			}

			if (!HTTP::WriteDirectoryEntries(ex, writer, _serverAddress, _file.path(), _appPath, sortField, sortOrder))
				writeError(500, "List folder files, ",ex.error());
		} else {
			/// File
#if defined(_WIN32)
			wchar_t wFile[_MAX_PATH];
			MultiByteToWideChar(CP_UTF8, 0, _file.path().c_str(), -1, wFile, _MAX_PATH);
			ifstream ifile(wFile, ios::in | ios::binary | ios::ate);
#else
			ifstream ifile(_file.path(), ios::in | ios::binary | ios::ate);
#endif
			if (!ifile.good())
				writeError(423, "Impossible to open ", _appPath, '/', _file.name(), " file");
			else {
				// determine the content-type
				string subType;
				HTTP::ContentType type = HTTP::ExtensionToMIMEType(_file.extension(), subType);	

				PacketWriter& response(write("200 OK", type, subType.c_str(), NULL, 2).packet); // 2 to impose a raw serialization
				HTTP_BEGIN_HEADER(response)
					HTTP_ADD_HEADER("Last-Modified", date.toString(Date::HTTP_FORMAT, _buffer))
				HTTP_END_HEADER

				// TODO see if filter is correct
				if (type == HTTP::CONTENT_TEXT && _pFileParams->count())
					replaceTemplateTags(response, ifile, *_pFileParams);
				else {

					// push the entire file content to memory
					UInt32 size = (UInt32)ifile.tellg();
					ifile.seekg(0);
					char* current = (char*)response.buffer(size); // reserve memory for file
					ifile.read(current, size);
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
			if (memcmp(++content, EXPAND("\r\n\r\n")) == 0) {
				content += 4;
				break;
			}
			if (content == end)
				ERROR("HTTP header without end, invalid packet")
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
	Session::DumpResponse("HTTP",data(), size(), _address);

	/// Send
	return TCPSender::run(ex);
}


void HTTPSender::writeFile(const string& path, const shared_ptr<Parameters>& pParameters) {
	_newHeaders = true;
	_file.setPath(path);
	_pFileParams = pParameters;
}

DataWriter& HTTPSender::writeResponse(const char* code, bool rawWithoutLength) {
	if (_request.contentType == HTTP::CONTENT_ABSENT)
		return write(code, HTTP::CONTENT_APPLICATON, "json", NULL, rawWithoutLength ? 1 : 0);
	return write(code, _request.contentType, _request.contentSubType.c_str(), NULL, rawWithoutLength ? 1 : (MIME::DataType(_request.contentSubType.c_str())==MIME::UNKNOWN  ? 2 : 0));
}

BinaryWriter& HTTPSender::writeRaw() {
	if (_pWriter) {
		ERROR("HTTP response already written");
		return DataWriter::Null.packet;
	}
	_pWriter.reset(new StringWriter(_poolBuffers));
	_connection = HTTP::CONNECTION_KEEPALIVE; // write content (no new header), keepalive the connection!
	return _pWriter->packet;
}


DataWriter& HTTPSender::write(const char* code, HTTP::ContentType type, const char* subType, const UInt8* data, UInt32 size) {
	if (_pWriter) {
		ERROR("HTTP response already written");
		return DataWriter::Null;
	}

	_newHeaders = true;

	if (type == HTTP::CONTENT_ABSENT || data || size || !subType || !MIME::CreateDataWriter(MIME::DataType(subType), _poolBuffers, _pWriter))
		_pWriter.reset(new StringWriter(_poolBuffers));

	PacketWriter& packet = _pWriter->packet;
	packet.clear();

	// First line (HTTP/1.1 200 OK)
	UInt16 value(200);
	packet.write(EXPAND("HTTP/1.1 "));
	packet.write(code);
	if (String::ToNumber<UInt16>(code, value)) {
		packet.write(" ");
		packet.write(HTTP::CodeToMessage(value));
	}

	// Date + Mona
	packet.write(EXPAND("\r\nDate: ")).write(Date().toString(Date::HTTP_FORMAT, _buffer));
	packet.write(EXPAND("\r\nServer: Mona"));

	// Connection type, same than request!
	if (_connection&HTTP::CONNECTION_KEEPALIVE) {
		packet.write(EXPAND("\r\nConnection: keep-alive"));
		if (_connection&HTTP::CONNECTION_UPGRADE)
			packet.write(EXPAND(", upgrade"));
		if (_connection&HTTP::CONNECTION_CLOSE)
			packet.write(EXPAND(", close"));
	} else if (_connection&HTTP::CONNECTION_UPGRADE) {
		packet.write(EXPAND("\r\nConnection: upgrade"));
		if (_connection&HTTP::CONNECTION_CLOSE)
			packet.write(EXPAND(", close"));
	} else if (_connection&HTTP::CONNECTION_CLOSE)
		packet.write(EXPAND("\r\nConnection: close"));

	// allow cross request, indeed if onConnection has not been rejected, every cross request are allowed
	if (String::ICompare(_origin, _serverAddress) != 0)
		packet.write(EXPAND("\r\nAccess-Control-Allow-Origin: ")).write(_origin);

	// Set Cookies
	if (_pSetCookieBuffer)
		packet.write(*_pSetCookieBuffer);

	// Content Type
	if (type == HTTP::CONTENT_ABSENT)
		packet.write(EXPAND("\r\nContent-Length: 0"));
	else {
		packet.write(EXPAND("\r\nContent-Type: "));
		packet.write(HTTP::FormatContentType(type, subType, _buffer));
		
		// Content Length
		if (data) {
			packet.write(EXPAND("\r\nContent-Length: "));
			packet.write(String::Format(_buffer, size));
		} else if (size == 1) { // if size==1 means that we want not writing a content-length
			// here it means that we are on a live streaming, without size limit, so we have to signal the cache-control
			packet.write(EXPAND("\r\nCache-Control: no-cache, no-store\r\nPragma: no-cache"));
			_connection = HTTP::CONNECTION_KEEPALIVE; // no content-length, keepalive the connection!
		} else {
			// reserve place to add length on sending
			packet.write(EXPAND("\r\nContent-Length:           "));
			_sizePos = packet.size()-10;
		}
	}

	packet.write("\r\n\r\n");
	if (data && size > 0)
		packet.write(data, size);
	if(data)
		return DataWriter::Null;
	_pWriter->clear(packet.size());
	return *_pWriter;
}


void HTTPSender::replaceTemplateTags(PacketWriter& packet, ifstream& ifile, const Parameters& parameters) {

	UInt32 pos = packet.size();
	// get file content size
	UInt32 size = (UInt32)ifile.tellg();

	// push the entire file content to memory
	ifile.seekg(0);
	char* current = (char*)packet.buffer(size+parameters.bytes()); // reserve more memory to change <%name%> field
	ifile.read(current, size);
	// iterate on content to replace "<% key %>" fields
	const char* begin = current;
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
					memmove(current+1+available,current+1,end-current-1);
					end += available;
					current += available-1;
				} else if (available>value.size()) {
					available = available-value.size(); // to remove
                    memmove(current+1-available,current+1,end-current-1);
					end -= available;
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
	packet.clear(pos+(end-begin));
}

} // namespace Mona
