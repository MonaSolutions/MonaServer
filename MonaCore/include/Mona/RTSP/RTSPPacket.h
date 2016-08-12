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
#include "Mona/RTSP/RTSP.h"
#include "Mona/HTTP/HTTP.h"
#include "Mona/File.h"


namespace Mona {

class RTSPPacket : public virtual Object, public Binary {
private:
	struct CmpStr {
		bool operator()(char const*a,char const*b) const { return std::strcmp(a,b) < 0; }
	};

public:
	

	RTSPPacket(const std::string& rootPath);

	const UInt8*	data() const { return _data; }
	UInt32			size() const { return _size; }

	Exception					exception;

	std::map<const char*,const char*,CmpStr>	headers;

	RTSP::CommandType			command;
	std::string					url;
	std::string					path;
	File						file;
	std::string					query;
	std::string					serverAddress;
	float						version;
	UInt16						cSeq;
	UInt8						trackID;


	HTTP::ContentType			responseType;

	UInt32						build(Exception& ex,UInt8* data,UInt32 size);

	void parseHeader(Exception& ex,const char* key, const char* value);
private:

	// for header reading
	enum ReadingStep {
		CMD,
		PATH,
		VERSION,
		LEFT,
		RIGHT
	};

	const UInt8*		_data;
	UInt32				_size;
};



} // namespace Mona
