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

#include "Mona/HTTP/HTTP.h"
#include "Mona/Logs.h"
#include "Mona/Util.h"

using namespace std;


namespace Mona {

static map<UInt16, string> CodeMessages({
	{ 100, HTTP_CODE_100 },
	{ 101, HTTP_CODE_101 },
	{ 102, HTTP_CODE_102 },
	{ 118, HTTP_CODE_118 },

	{ 200, HTTP_CODE_200 },
	{ 201, HTTP_CODE_201 },
	{ 202, HTTP_CODE_202 },
	{ 203, HTTP_CODE_203 },
	{ 204, HTTP_CODE_204 },
	{ 205, HTTP_CODE_205 },
	{ 206, HTTP_CODE_206 },
	{ 207, HTTP_CODE_207 },
	{ 210, HTTP_CODE_210 },
	{ 226, HTTP_CODE_226 },

	{ 300, HTTP_CODE_300 },
	{ 301, HTTP_CODE_301 },
	{ 302, HTTP_CODE_302 },
	{ 303, HTTP_CODE_303 },
	{ 304, HTTP_CODE_304 },
	{ 305, HTTP_CODE_305 },
	{ 307, HTTP_CODE_307 },
	{ 310, HTTP_CODE_310 },

	{ 400, HTTP_CODE_400 },
	{ 401, HTTP_CODE_401 },
	{ 402, HTTP_CODE_402 },
	{ 403, HTTP_CODE_403 },
	{ 404, HTTP_CODE_405 },
	{ 405, HTTP_CODE_405 },
	{ 406, HTTP_CODE_406 },
	{ 407, HTTP_CODE_407 },
	{ 408, HTTP_CODE_408 },
	{ 409, HTTP_CODE_409 },
	{ 410, HTTP_CODE_410 },
	{ 411, HTTP_CODE_411 },
	{ 412, HTTP_CODE_412 },
	{ 413, HTTP_CODE_413 },
	{ 414, HTTP_CODE_414 },
	{ 415, HTTP_CODE_415 },
	{ 416, HTTP_CODE_416 },
	{ 417, HTTP_CODE_417 },
	{ 418, HTTP_CODE_418 },
	{ 422, HTTP_CODE_422 },
	{ 423, HTTP_CODE_423 },
	{ 424, HTTP_CODE_424 },
	{ 425, HTTP_CODE_425 },
	{ 426, HTTP_CODE_426 },
	{ 428, HTTP_CODE_428 },
	{ 429, HTTP_CODE_429 },
	{ 431, HTTP_CODE_431 },
	{ 449, HTTP_CODE_449 },
	{ 450, HTTP_CODE_450 },
	{ 456, HTTP_CODE_456 },
	{ 499, HTTP_CODE_499 },

	{ 500, HTTP_CODE_500 },
	{ 501, HTTP_CODE_501 },
	{ 502, HTTP_CODE_502 },
	{ 503, HTTP_CODE_503 },
	{ 504, HTTP_CODE_504 },
	{ 505, HTTP_CODE_505 },
	{ 506, HTTP_CODE_506 },
	{ 507, HTTP_CODE_507 },
	{ 508, HTTP_CODE_508 },
	{ 509, HTTP_CODE_509 },
	{ 510, HTTP_CODE_510 },
	{ 520, HTTP_CODE_520 }
});

void HTTP::MIMEType(const string& extension, string& type) {
	if (String::ICompare(extension, "svg") == 0)
		type = "image/svg+xml";
	else if (String::ICompare(extension, "js") == 0)
		type = "application/javascript";
	else {
		String::Format(type, "text/", extension);
	} // TODO!!
}

void HTTP::ReadHeader(HTTPPacketReader& reader, MapParameters& headers, string& cmd, string& path, string& file, MapParameters& properties) {
	HTTPPacketReader::Type type;
	bool first = true;
	Exception ex;
	while ((type = reader.followingType()) != HTTPPacketReader::END) {
		if (type == HTTPPacketReader::STRING) {
			string value;
			reader.readString(value);
			if (first) {
				first = false;
				vector<string> fields;
				String::Split(value, " ", fields, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
				if (fields.size() > 0) {
					cmd = fields[0];
					if (fields.size() > 1 && Util::UnpackUrl(ex,fields[1], path, file, properties)) {
						if (fields.size() > 2) {
							unsigned found = fields[2].find_last_of("/");
							Exception ex;
							double value = String::ToNumber<double>(ex, fields[2].substr(found + 1));
							if (found != string::npos && !ex)
								properties.setNumber("HTTPVersion", value); // TODO check how is named for AMF
						}
					}
					if (ex)
						WARN("HTTPHeader malformed, ", ex.error())
				}
			}
			continue;
		}

		if (type != HTTPPacketReader::OBJECT) {
			reader.next();
			continue;
		}

		string name, value;
		while ((type = reader.readItem(name)) != HTTPPacketReader::END) {
			if (type != HTTPPacketReader::STRING) {
				reader.next();
				continue;
			}
			if (String::ToLower(name).compare("referer") == 0) {
				string referer;
				Util::UnpackUrl(ex,reader.readString(referer), path, properties);
				if (ex)
					WARN("HTTPHeader malformed, ", ex.error())
			}
			headers.setString(name, reader.readString(value));
		}
	}
}


void HTTP::CodeToMessage(UInt16 code, std::string& message) {
	auto found = CodeMessages.find(code);
	if (found != CodeMessages.end())
		message = found->second;
}



} // namespace Mona
