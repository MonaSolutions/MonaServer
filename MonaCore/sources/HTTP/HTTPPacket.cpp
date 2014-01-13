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
#include "Mona/Logs.h"

using namespace std;


namespace Mona {


HTTPPacket::HTTPPacket(PoolBuffer& pBuffer) : _pBuffer(pBuffer),
	content(NULL),
	contentLength(0),
	contentType(HTTP::CONTENT_ABSENT),
	command(HTTP::COMMAND_HEAD),// default value, less instrusive
	version(0),
	connection(HTTP::CONNECTION_ABSENT),
	ifModifiedSince(0),
	accessControlRequestMethod(0) {

}

void HTTPPacket::parseHeader(const char* key, const char* value) {
	if (String::ICompare(key,"content-length")==0) {
		Exception ex;
		contentLength = String::ToNumber<UInt32>(ex,value,contentLength);
	} else if (String::ICompare(key,"content-type")==0) {
		contentType = HTTP::ParseContentType(value, contentSubType);
	} else if (String::ICompare(key,"connection")==0) {
		connection = HTTP::ParseConnection(value);
	} else if (String::ICompare(key,"host")==0) {
		serverAddress.assign(value);
	} else if (String::ICompare(key,"upgrade")==0) {
		upgrade.assign(value);
	} else if (String::ICompare(key,"sec-websocket-key")==0) {
		secWebsocketKey.assign(value);
	} else if (String::ICompare(key,"sec-webSocket-accept")==0) {
		secWebsocketAccept.assign(value);
	} else if (String::ICompare(key,"if-modified-since")==0) {
		ifModifiedSince.fromString(value);
	} else if (String::ICompare(key,"access-control-request-method")==0) {
		vector<string> values;
		for (string& value : String::Split(value, ",", values,String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM))
			accessControlRequestMethod |= HTTP::ParseCommand(value.c_str());
	}
}
	
const UInt8* HTTPPacket::build(PoolBuffer& pBuffer,const UInt8* data,UInt32& size) {
	/// append data
	if (!_pBuffer.empty()) {
		UInt32 oldSize = _pBuffer->size();
		_pBuffer->resize(oldSize+size,true);
		memcpy(_pBuffer->data()+oldSize, data,size);
	} else
		_pBuffer.swap(pBuffer); // exchange the buffers

	/// read data
	UInt8				newLineCount(0);
	ReadingStep			step(CMD);
	UInt8*				current(_pBuffer->data());
	UInt8*				end(current+_pBuffer->size());
	const char*			signifiant(NULL);
	const char*			key(NULL);

	for (current; current < end;++current) {
		// http header
		UInt8 byte = *current;

		if (byte=='\r') {
			++newLineCount;
			step = LINE_RETURN;
		} else if (step==LINE_RETURN && byte=='\n')
			++newLineCount;
		else {
			newLineCount = 0;

			if ((step==LEFT || step == CMD || step == PATH) && isspace(byte)) {
				if (step == CMD) {
					// by default command == GET
					command = HTTP::ParseCommand(signifiant);
					signifiant = NULL;
					step = PATH;
				} else if (step == PATH) {
					_buffer.assign(signifiant,(const char*)current-signifiant);
					// parse query
					filePos = Util::UnpackUrl(_buffer, path, parameters);
					signifiant = NULL;
					step = VERSION;
				} else
					++signifiant; // for trim begin of key or value
				continue;
			} else if (!key && byte == ':') {
				// KEY
				key = signifiant;
				step = LEFT;
				UInt8* prev = current;
				while (isblank(*--current));
				*(current+1) = '\0';
				current = prev;
				headers.emplace_back(signifiant);
				signifiant = (const char*)current + 1;
			} else if (step == CMD || step == PATH || step == VERSION) {
				if (!signifiant)
					signifiant = (const char*)current;
			} else
				step = RIGHT;
			continue;
		}
		
		if (newLineCount == 2) {
			if (signifiant) {
				// KEY = VALUE
				UInt8* prev = current--;
				while (isblank(*--current));
				*(current+1) = '\0';
				current = prev;
				if (!key) { // version case!
					String::ToNumber(signifiant+5, version);
				} else {
					headers.emplace_back(signifiant);
					parseHeader(key,signifiant);
					key = NULL;
				}
			}
			step = LEFT;
			signifiant = (const char*)current+1;
		} else if (newLineCount == 4) {
			content = current + 1;
			current += contentLength;
			break;
		}
	}

	if (current >= end)
		return NULL;  // wait next data

	// exchange the buffers
	pBuffer.swap(_pBuffer);

	UInt32 rest = end-current-1;
	if (rest == 0) {
		_pBuffer.release(); // release buffer
		size = pBuffer->size();
		return pBuffer->data();
	}

	// prepare next iteration
	_pBuffer->resize(rest,false);
	memcpy(_pBuffer->data(), current + 1, rest);
	

	// shrink data
	size = size-rest;
	return pBuffer->data();
}


} // namespace Mona
