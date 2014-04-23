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
#include "Mona/MapWriter.h"
#include "Mona/HTTP/HTTP.h"


namespace Mona {

class HTTPPacket : public virtual Object {
public:

	HTTPPacket(PoolBuffer& pBuffer);

	Exception					exception;

	std::vector<const char*>	headers;
	const UInt8*				content;
	UInt32						contentLength;
	HTTP::ContentType			contentType;
	std::string					contentSubType;

	HTTP::CommandType			command;
	std::string					path;
	std::string					query;
	std::string					serverAddress;
	float						version;
	std::size_t					filePos;

	UInt8						connection;
	std::string					upgrade;
	UInt8						cacheControl;
	
	Date						ifModifiedSince;
	UInt8						accessControlRequestMethod;

	std::string					secWebsocketKey;
	std::string					secWebsocketAccept;

	MapWriter<std::map<std::string,std::string>>	parameters; // For onRead returned value (return file,parameters)
	
	const PoolBuffers&			poolBuffers() { return _pBuffer.poolBuffers; }


	const UInt8*				build(Exception& ex,PoolBuffer& pBuffer,const UInt8* data,UInt32& size);

private:
	void parseHeader(Exception& ex,const char* key, const char* value);

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
