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


HTTPPacket::HTTPPacket(const shared_ptr<PoolBuffer>& ppBuffer) : filePos(string::npos), _ppBuffer(ppBuffer),
	content(NULL),
	contentLength(0),
	contentType(HTTP::CONTENT_ABSENT),
	command(HTTP::COMMAND_UNKNOWN),
	version(0),
	connection(HTTP::CONNECTION_ABSENT),
	ifModifiedSince(0),
	accessControlRequestMethod(0),
	rawSerialization(false) {

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
	} else if (String::ICompare(key,"upgrade")==0) {
		upgrade.assign(value);
	} else if (String::ICompare(key,"sec-websocket-key")==0) {
		secWebsocketKey.assign(value);
	} else if (String::ICompare(key,"sec-webSocket-accept")==0) {
		secWebsocketAccept.assign(value);
	} else if (String::ICompare(key,"if-modified-since")==0) {
		ifModifiedSince.update(ex,value,Date::HTTP_FORMAT);
	} else if (String::ICompare(key,"access-control-request-method")==0) {

		String::ForEach forEach([this,&ex](UInt32 index,const char* value){
			accessControlRequestMethod |= HTTP::ParseCommand(ex,value);
			return true;
		});
		String::Split(value, ",", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);

	} else if (String::ICompare(key,"cookie")==0) {
		cookies.assign(value);
	}
}
	
const UInt8* HTTPPacket::build(Exception& ex,PoolBuffer& pBuffer,const UInt8* data,UInt32& size) {

	/// append data
	if (!_ppBuffer->empty()) {
		UInt32 oldSize = (*_ppBuffer)->size();
		(*_ppBuffer)->resize(oldSize+size,true);
		memcpy((*_ppBuffer)->data()+oldSize, data,size);
	} else
		_ppBuffer->swap(pBuffer); // exchange the buffers

	/// read data
	UInt8				newLineCount(0);
	ReadingStep			step(CMD);
	UInt8*				current((*_ppBuffer)->data());
	UInt8*				end(current+(*_ppBuffer)->size());
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

			if ((step == LEFT || step == CMD || step == PATH) && isspace(byte)) {
				if (step == CMD) {
					// by default command == GET
					if ((command = HTTP::ParseCommand(ex, signifiant)) == HTTP::COMMAND_UNKNOWN) {
						_ppBuffer->release();
						exception.set(ex);
						return NULL;
					}
					signifiant = NULL;
					step = PATH;
				} else if (step == PATH) {
					// parse query
					*current = 0;
					filePos = Util::UnpackUrl(signifiant, path,query);
					signifiant = NULL;
					step = VERSION;
				} else
					++signifiant; // for trim begin of key or value
				continue;
			} else if (step != CMD && !key && byte == ':') {
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
				if (step == CMD && (current-(*_ppBuffer)->data())>7) {
					// not a HTTP valid packet, consumes all
					_ppBuffer->release();
					exception.set(ex.set(Exception::PROTOCOL, "invalid HTTP packet"));
					return NULL;
				}
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
					parseHeader(ex,key,signifiant);
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
	pBuffer.swap(*_ppBuffer);

	UInt32 rest = end-current-1;
	if (rest == 0) {
		_ppBuffer->release(); // release buffer
		size = pBuffer->size();
		return pBuffer->data();
	}

	// prepare next iteration
	(*_ppBuffer)->resize(rest,false);
	memcpy((*_ppBuffer)->data(), current + 1, rest);
	

	// shrink data
	size = size-rest;
	return pBuffer->data();
}


} // namespace Mona
