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

#pragma once

#include "Mona/Mona.h"
#include "Mona/PoolBuffer.h"
#include "Mona/Time.h"
#include "Mona/MapParameters.h"
#include "Mona/MapWriter.h"
#include "Mona/HTTP/HTTP.h"
#include <deque>

namespace Mona {

class HTTPPacket : virtual Object {
public:

	HTTPPacket(PoolBuffer& pBuffer);

	std::deque<const char*>		headers;
	const UInt8*				content;
	UInt32						contentLength;
	HTTP::ContentType			contentType;
	std::string					contentSubType;

	
	HTTP::CommandType			command;
	std::string					path;
	std::size_t					filePos;
	MapParameters				parameters;
	float						version;

	UInt8						connection;
	std::string					upgrade;
	UInt8						cacheControl;
	std::string					serverAddress;
	Time						ifModifiedSince;
	UInt8						accessControlRequestMethod;

	std::string					secWebsocketKey;
	std::string					secWebsocketAccept;

	MapWriter<std::map<std::string,std::string>>	properties;

	const PoolBuffers&			poolBuffers() { return _pBuffer.poolBuffers; }


	const UInt8*				build(PoolBuffer& pBuffer,const UInt8* data,UInt32& size);

private:
	void parseHeader(const char* key, const char* value);

	// for header
	enum ReadingStep {
		CMD,
		PATH,
		VERSION,
		LEFT,
		RIGHT,
		LINE_RETURN,
	};

	PoolBuffer&	_pBuffer;
	std::string	_buffer;
};




} // namespace Mona
