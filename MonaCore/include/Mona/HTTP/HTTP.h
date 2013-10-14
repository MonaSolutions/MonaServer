/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
#include "Mona/HTTPPacketReader.h"
#include "Mona/MapParameters.h"
#include <map>

namespace Mona {

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
#define HTTP_CODE_418	"I’m a teapot"
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


class HTTP {
public:
	static void 	MIMEType(const std::string& extension,std::string& type);
	static void 	CodeToMessage(Poco::UInt16 code,std::string& message);
	static void 	ReadHeader(HTTPPacketReader& reader, MapParameters& headers, std::string& cmd, std::string& path, std::string& file, MapParameters& properties);

private:
	static std::map<Poco::UInt16,std::string>	_CodeMessages;
};




} // namespace Mona
