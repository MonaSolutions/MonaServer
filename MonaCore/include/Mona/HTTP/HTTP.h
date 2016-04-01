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
#include "Mona/BinaryWriter.h"
#include "Mona/MapParameters.h"
#include "Mona/File.h"
#include "Mona/DataWriter.h"
#include "Mona/DataReader.h"

namespace Mona {


#define HTML_BEGIN_COMMON_RESPONSE(BINARYWRITER,TITLE) \
	{ BinaryWriter& __binary(BINARYWRITER); __binary.write(EXPAND("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>")).write(TITLE).write(EXPAND("</title></head><body><h1>")).write(TITLE).write(EXPAND("</h1><p>"));

#define HTML_END_COMMON_RESPONSE(ADDRESS) \
	__binary.write(EXPAND("</p><hr><address>Mona Server at ")).write(ADDRESS).write(EXPAND("</address></body></html>\r\n")); }


#define HTTP_BEGIN_HEADER(BINARYWRITER)  { BinaryWriter& __binary(BINARYWRITER);  __binary.clear(__binary.size()-2);
#define HTTP_ADD_HEADER(NAME,VALUE) { __binary.write(EXPAND(NAME)).write(": ").write(VALUE).write("\r\n"); }
#define HTTP_END_HEADER  __binary.write("\r\n");  }



#define HTTP_CODE_100	"Continue"
#define HTTP_CODE_101	"Switching Protocols"
#define HTTP_CODE_102	"Processing"
#define HTTP_CODE_118	"Connection timed out"
#define HTTP_CODE_200	"OK"
#define HTTP_CODE_201	"Created"
#define HTTP_CODE_202	"Accepted"
#define HTTP_CODE_203	"Non-Authoritative Information"
#define HTTP_CODE_204	"No Content"
#define HTTP_CODE_205	"Reset Content"
#define HTTP_CODE_206	"Partial Content"
#define HTTP_CODE_207	"Multi-Status"
#define HTTP_CODE_210	"Content Different"
#define HTTP_CODE_226	"IM Used"

#define HTTP_CODE_300	"Multiple Choices"
#define HTTP_CODE_301	"Moved Permanently"
#define HTTP_CODE_302	"Moved Temporarily"
#define HTTP_CODE_303	"See Other"
#define HTTP_CODE_304	"Not Modified"
#define HTTP_CODE_305	"Use Proxy"
#define HTTP_CODE_307	"Temporary Redirect"
#define HTTP_CODE_310	"Too many Redirects"

#define HTTP_CODE_400	"Bad Request"
#define HTTP_CODE_401	"Unauthorized"
#define HTTP_CODE_402	"Payment Required"
#define HTTP_CODE_403	"Forbidden"
#define HTTP_CODE_404	"Not Found"
#define HTTP_CODE_405	"Method Not Allowed"
#define HTTP_CODE_406	"Not Acceptable"
#define HTTP_CODE_407	"Proxy Authentication Required"
#define HTTP_CODE_408	"Request Time-out"
#define HTTP_CODE_409	"Conflict"
#define HTTP_CODE_410	"Gone"
#define HTTP_CODE_411	"Length Required"
#define HTTP_CODE_412	"Precondition Failed"
#define HTTP_CODE_413	"Request Entity Too Large"
#define HTTP_CODE_414	"Request-URI Too Long"
#define HTTP_CODE_415	"Unsupported Media Type"
#define HTTP_CODE_416	"Requested range unsatisfiable"
#define HTTP_CODE_417	"Expectation failed"
#define HTTP_CODE_418	"I'm a teapot"
#define HTTP_CODE_422	"Unprocessable entity"
#define HTTP_CODE_423	"Locked"
#define HTTP_CODE_424	"Method failure"
#define HTTP_CODE_425	"Unordered Collection"
#define HTTP_CODE_426	"Upgrade Required"
#define HTTP_CODE_428	"Precondition Required"
#define HTTP_CODE_429	"Too Many Requests"
#define HTTP_CODE_431	"Request Header Fields Too Large"
#define HTTP_CODE_449	"Retry With"
#define HTTP_CODE_450	"Blocked by Windows Parental Controls"
#define HTTP_CODE_456	"Unrecoverable Error"
#define HTTP_CODE_499	"client has closed connection"

#define HTTP_CODE_500	"Internal Server Error"
#define HTTP_CODE_501	"Not Implemented"
#define HTTP_CODE_502	"Proxy Error"
#define HTTP_CODE_503	"Service Unavailable"
#define HTTP_CODE_504	"Gateway Time-out"
#define HTTP_CODE_505	"HTTP Version not supported"
#define HTTP_CODE_506	"Variant also negociate"
#define HTTP_CODE_507	"Insufficient storage"
#define HTTP_CODE_508	"Loop detected"
#define HTTP_CODE_509	"Bandwidth Limit Exceeded"
#define HTTP_CODE_510	"Not extended"
#define HTTP_CODE_520	"Web server is returning an unknown error"

class HTTPSession;
class PacketReader;

class HTTP : virtual Static {
public:


	enum CommandType {
		COMMAND_UNKNOWN = 0,
		COMMAND_HEAD=1,
		COMMAND_GET = 2,
		COMMAND_PUSH = 4,
		COMMAND_OPTIONS = 8,
		COMMAND_POST = 16,
		COMMAND_DELETE = 32
	};

	enum ContentType {
		CONTENT_ABSENT=0,
		CONTENT_TEXT,
		CONTENT_APPLICATON,
		CONTENT_EXAMPLE,
		CONTENT_AUDIO,
		CONTENT_VIDEO,
		CONTENT_IMAGE,
		CONTENT_MESSAGE,
		CONTENT_MODEL,
		CONTENT_MULTIPART
	};

	enum ConnectionType {
		CONNECTION_ABSENT = 0,
		CONNECTION_CLOSE = 1,
		CONNECTION_UPGRADE = 2,
		CONNECTION_KEEPALIVE = 4,
		CONNECTION_UPDATE = 8
	};

	enum SortOrder {
		SORT_ASC,
		SORT_DESC,
	};

	enum SortField {
		SORT_BY_NAME,
		SORT_BY_MODIFIED,
		SORT_BY_SIZE
	};

	static CommandType	ParseCommand(Exception& ex,const char* value);
	static ContentType	ParseContentType(const char* value,std::string& subType);
	static UInt8		ParseConnection(Exception& ex,const char* value);

	static std::string&	FormatContentType(ContentType type,const char* subType,std::string& value);

	static ContentType	ExtensionToMIMEType(const std::string& extension, std::string& subType);

	static const char*	CodeToMessage(UInt16 code);

	static bool			WriteDirectoryEntries(Exception& ex, BinaryWriter& writer, const std::string& serverAddress, const std::string& fullPath, const std::string& path, SortField sortField = SORT_BY_NAME, SortOrder sortOrder = SORT_ASC);

	typedef std::function<void(const char* key,const char* data,UInt32 size)> OnCookie;
	static bool			WriteSetCookie(DataReader& reader,Buffer& buffer,const OnCookie& onCookie=nullptr);

private:
	static void			WriteDirectoryEntry(BinaryWriter& writer, const std::string& serverAddress,const std::string& path,const File& entry);
};




} // namespace Mona
