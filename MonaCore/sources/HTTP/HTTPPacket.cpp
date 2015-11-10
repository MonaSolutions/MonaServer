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

#include "Mona/HTTP/HTTPPacket.h"
#include "Mona/Util.h"

using namespace std;


namespace Mona {


HTTPPacket::HTTPPacket(const string& rootPath) : accessControlRequestMethod(0), accessControlRequestHeaders(NULL),
	file(rootPath), _data(NULL), _size(0),
	content(NULL),
	contentLength(0),
	contentType(HTTP::CONTENT_ABSENT),
	command(HTTP::COMMAND_UNKNOWN),
	version(0),
	connection(HTTP::CONNECTION_ABSENT),
	ifModifiedSince(0) {

}

void HTTPPacket::parseHeader(Exception& ex,const char* key, const char* value) {
	if (String::ICompare(key,"content-length")==0) {
		contentLength = String::ToNumber(ex,contentLength, value);
	} else if (String::ICompare(key,"content-type")==0) {
		contentType = HTTP::ParseContentType(value, contentSubType);
	} else if (String::ICompare(key,"connection")==0) {
		connection = HTTP::ParseConnection(ex,value);
	} else if (String::ICompare(key,"host")==0) {
		serverAddress.assign(value);
	} else if (String::ICompare(key,"origin")==0) {
		origin.assign(value);
	} else if (String::ICompare(key,"upgrade")==0) {
		upgrade.assign(value);
	} else if (String::ICompare(key,"sec-websocket-key")==0) {
		secWebsocketKey.assign(value);
	} else if (String::ICompare(key,"sec-webSocket-accept")==0) {
		secWebsocketAccept.assign(value);
	} else if (String::ICompare(key,"if-modified-since")==0) {
		ifModifiedSince.update(ex,value,Date::HTTP_FORMAT);
	} else if (String::ICompare(key,"access-control-request-headers")==0) {
		accessControlRequestHeaders = value;
	} else if (String::ICompare(key,"access-control-request-method")==0) {

		String::ForEach forEach([this,&ex](UInt32 index,const char* value){
			accessControlRequestMethod |= HTTP::ParseCommand(ex,value);
			return true;
		});
		String::Split(value, ",", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);

	} else if (String::ICompare(key,"cookie")==0) {
		String::ForEach forEach([this](UInt32 index,const char* data) {
			const char* value = data;
			// trim right
			while (value && *value != '=' && !isblank(*value))
				++value;
			if (value) {
				const char *endKey=value;
				// trim left
				do {
					++value;
				} while (value && (isblank(*value) || *value == '='));
				SCOPED_STRINGIFY(data, endKey-data, cookies[data] = value);
			}
			return true;
		});
		String::Split(value, ";", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
	}
}
	
UInt32 HTTPPacket::build(Exception& ex,UInt8* data,UInt32 size) {
	if (_data)
		return 0;
	exception.set(Exception::NIL);

	/// read data
	ReadingStep			step(CMD);
	UInt8*				current(data);
	const UInt8*		end(current+size-4); // 4 == /r/n/r/n
	const char*			signifiant(NULL);
	const char*			key(NULL);

	// headers

	for (; current <= end;++current) {

		if (memcmp(current, EXPAND("\r\n")) == 0 || memcmp(current, EXPAND("\0\n")) == 0) {

			if (!ex && signifiant) {
				// KEY = VALUE
				UInt8* endValue(current);
				while (isblank(*--endValue));
				*(endValue+1) = 0;
				if (!key) { // version case!
					String::ToNumber(signifiant+5, version);
				} else {
					headers[key] = signifiant;
					parseHeader(ex,key,signifiant);
					key = NULL;
				}
			}
			step = LEFT;
			current += 2;
			signifiant = (const char*)current;

			if (memcmp(current, EXPAND("\r\n")) == 0) {
				current += 2;
				content = current;
				current += contentLength;
				if (ex || current > (end+4))
					break; // wait next
				_data = data;
				return _size = current - data;
			}

			++current; // here no continue, the "\r\n" check is not required again
		}

		if (ex)
			continue; // try to go to "\r\n\r\n"

		// http header, byte by byte
		UInt8 byte = *current;

		if ((step == LEFT || step == CMD || step == PATH) && (isspace(byte) || byte==0)) {
			if (step == CMD) {
				if(!signifiant) // Space before signifiant
					ex.set(Exception::PROTOCOL,"Unexpected space before command");

				// by default command == GET
				if (!signifiant || (command = HTTP::ParseCommand(ex, signifiant)) == HTTP::COMMAND_UNKNOWN) {
					exception = ex;
					continue;
				}
				signifiant = NULL;
				step = PATH;
			} else if (step == PATH) {
				// parse query
				*current = 0;
				size_t filePos = Util::UnpackUrl(signifiant, path,query);
				file.setPath(file.path(),path);
				if (filePos != string::npos)
					path.erase(filePos - 1);
				else
					file.makeFolder();
				signifiant = NULL;
				step = VERSION;
			} else
				++signifiant; // for trim begin of key or value
		} else if (step > VERSION && !key && (byte == ':' || byte == 0)) {
			// KEY
			key = signifiant;
			step = LEFT;
			UInt8* endValue(current);
			while (isblank(*--endValue));
			*(endValue+1) = 0;
			signifiant = (const char*)current + 1;
		} else if (step == CMD || step == PATH || step == VERSION) {
			if (!signifiant)
				signifiant = (const char*)current;
			if (step == CMD && (current-data)>7) // not a HTTP valid packet, consumes all
				exception = ex.set(Exception::PROTOCOL, "invalid HTTP packet");
		} else
			step = RIGHT;
	}


	if (ex)
		return current - data;

	return 0;  // wait next data
}


} // namespace Mona
