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
#include "Mona/HTTP/HTTP.h"


namespace Mona {

class HTTPSendingInfos : public virtual Object {
public:
	HTTPSendingInfos() : sizeParameters(0),accessControlRequestMethod(0) {}
	std::vector<std::string>	setCookies; /// List of Set-cookie lines to add
	MapParameters				parameters; // For onRead returned value (return file,parameters)
	UInt32						sizeParameters;

	UInt8						accessControlRequestMethod;
	std::string					accessControlRequestHeaders;
};

class HTTPPacket : public virtual Object, public Binary {
public:
	

	HTTPPacket(const std::string& rootPath);

	const UInt8*	data() const { return _data; }
	UInt32			size() const { return _size; }

	Exception					exception;

	std::map<const char*,const char*>	headers;
	const UInt8*						content;
	UInt32								contentLength;
	HTTP::ContentType					contentType;
	std::string							contentSubType;
	bool								rawSerialization;

	HTTP::CommandType			command;
	std::string					path;
	Path						filePath;
	std::string					query;
	std::string					serverAddress;
	float						version;
	std::string					origin;

	UInt8						connection;
	std::string					upgrade;
	UInt8						cacheControl;
	
	Date						ifModifiedSince;

	std::string					secWebsocketKey;
	std::string					secWebsocketAccept;

	std::map<std::string,std::string>	cookies; /// List of cookie key;value

	UInt32						build(Exception& ex,UInt8* data,UInt32 size);

	HTTPSendingInfos&					sendingInfos() { if (!_pSendingInfos) _pSendingInfos.reset(new HTTPSendingInfos()); return *_pSendingInfos; }
	std::shared_ptr<HTTPSendingInfos>	pullSendingInfos() { return std::move(_pSendingInfos); }
	

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

	// For next HTTPSender
	std::shared_ptr<HTTPSendingInfos>	_pSendingInfos;

	const UInt8*		_data;
	UInt32				_size;
};



} // namespace Mona
