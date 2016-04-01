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

#include "Mona/HTTP/HTTP.h"
#include "Mona/FileSystem.h"
#include "Mona/Util.h"
#include <algorithm>

#include "Mona/HTTP/HTTPSession.h"
#include "Mona/PacketReader.h"
#include "Mona/JSONReader.h"
#include "Mona/XMLRPCReader.h"

using namespace std;


namespace Mona {


static const map<UInt16, const char*> CodeMessages({
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
	{ 404, HTTP_CODE_404 },
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


HTTP::ContentType HTTP::ExtensionToMIMEType(const string& extension, string& subType) {
	// TODO Make a Static Array and complete extensions
	if (String::ICompare(extension, "js") == 0) {
		subType = "javascript";
		return CONTENT_APPLICATON;
	}
	else if (String::ICompare(extension, "flv") == 0) {
		subType = "x-flv";
		return CONTENT_VIDEO;
	}
	else if (String::ICompare(extension, "ts") == 0) {
		subType = "mp2t";
		return CONTENT_VIDEO;
	}
	else if (String::ICompare(extension, "svg") == 0) {
		subType = "svg+xml";
		return CONTENT_IMAGE;
	}
	else if (String::ICompare(extension, EXPAND("m3u")) == 0) {
		subType = (extension.size() > 3 && extension[3] == '8') ? "x-mpegurl; charset=utf-8" : "x-mpegurl";
		return CONTENT_AUDIO;
	}
	else if (String::ICompare(extension, "jpg") == 0 || String::ICompare(extension, "jpeg") == 0) {
		subType = "jpeg";
		return CONTENT_IMAGE;
	}
	else if (String::ICompare(extension, "swf") == 0) {
		subType = "x-shockwave-flash";
		return CONTENT_APPLICATON;
	}

	subType = extension.empty() ? "plain" : extension;
	return CONTENT_TEXT;
}


string& HTTP::FormatContentType(ContentType type, const char* subType, string& value) {
	switch (type) {
		case CONTENT_TEXT:
			value.assign("text/");
			if (!subType)
				subType = "plain";
			break;
		case CONTENT_IMAGE:
			value.assign("image/");
			if (!subType)
				subType = "jpeg";
			break;
		case CONTENT_APPLICATON:
			value.assign("application/");
			if (!subType)
				subType = "octet-stream";
			break;
		case CONTENT_MULTIPART:
			value.assign("multipart/");
			if (!subType)
				subType = "mixed";
			break;
		case CONTENT_AUDIO:
			value.assign("audio/");
			if (!subType)
				subType = "mp2t";
			break;
		case CONTENT_VIDEO:
			value.assign("video/");
			if (!subType)
				subType = "mp2t";
			break;
		case CONTENT_MESSAGE:
			value.assign("message/");
			if (!subType)
				subType = "example";
			break;
		case CONTENT_MODEL:
			value.assign("model/");
			if (!subType)
				subType = "example";
			break;
		case CONTENT_EXAMPLE:
			value.assign("example");
			subType = "";
			break;
		default:
			return value.assign("text/plain");
	}
	return value.append(subType);
}

const char* HTTP::CodeToMessage(UInt16 code) {
	auto found = CodeMessages.find(code);
	if (found != CodeMessages.end())
		return found->second;
	return "Unknown";
}

HTTP::CommandType HTTP::ParseCommand(Exception& ex,const char* value) {
	if (String::ICompare(value,EXPAND("GET"))==0)
		return COMMAND_GET;
	if (String::ICompare(value,EXPAND("PUSH"))==0)
		return COMMAND_PUSH;
	if (String::ICompare(value,EXPAND("HEAD"))==0)
		return COMMAND_HEAD;
	if (String::ICompare(value,EXPAND("OPTIONS"))==0)
		return COMMAND_OPTIONS;
	if (String::ICompare(value,EXPAND("POST"))==0)
		return COMMAND_POST;
	ex.set(Exception::PROTOCOL, "Unknown HTTP command ", string(value, 4));
	return COMMAND_UNKNOWN;
}

UInt8 HTTP::ParseConnection(Exception& ex,const char* value) {
	UInt8 type(CONNECTION_ABSENT);
	String::ForEach forEach([&type,&ex](UInt32 index,const char* field){
		if (String::ICompare(field, "upgrade") == 0)
			type |= CONNECTION_UPGRADE;
		else if (String::ICompare(field, "keep-alive") == 0)
			type |= CONNECTION_KEEPALIVE;
		else if (String::ICompare(field, "close") == 0)
			type |= CONNECTION_CLOSE;
		else if (String::ICompare(field, "update") == 0)
			type |= CONNECTION_UPDATE;
		else
			ex.set(Exception::PROTOCOL, "Unknown HTTP type connection ", field);
		return true;
	});
	String::Split(value, ",", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
	return type;
}


HTTP::ContentType HTTP::ParseContentType(const char* value, string& subType) {
	
	// subtype
	const char* comma = strchr(value, ';');
	const char* slash = (const char*)memchr(value, '/', comma ? comma - value : strlen(value));
	if (slash)
		subType.assign(slash+1);
	else if (comma)
		subType.assign(comma);
	else
		subType.clear();

	// type
	if (String::ICompare(value,EXPAND("text"))==0)
		return CONTENT_TEXT;
	if (String::ICompare(value,EXPAND("image"))==0)
		return CONTENT_IMAGE;
	if (String::ICompare(value,EXPAND("application"))==0)
		return CONTENT_APPLICATON;
	if (String::ICompare(value,EXPAND("multipart"))==0)
		return CONTENT_MULTIPART;
	if (String::ICompare(value,EXPAND("audio"))==0)
		return CONTENT_AUDIO;
	if (String::ICompare(value,EXPAND("video"))==0)
		return CONTENT_VIDEO;
	if (String::ICompare(value,EXPAND("message"))==0)
		return CONTENT_MESSAGE;
	if (String::ICompare(value,EXPAND("model"))==0)
		return CONTENT_MODEL;
	if (String::ICompare(value,EXPAND("example"))==0)
		return CONTENT_EXAMPLE;
	return CONTENT_TEXT; // default value
}


struct EntriesComparator {
	EntriesComparator(HTTP::SortField field, HTTP::SortOrder order) : _field(field),_order(order) {}

	bool operator() (const File* pFile1,const File* pFile2) {
		if (_field == HTTP::SORT_BY_SIZE) {
			if (pFile1->size() != pFile2->size())
				return _order==HTTP::SORT_ASC ? pFile1->size() < pFile2->size() : pFile2->size() < pFile1->size();
		} else if (_field == HTTP::SORT_BY_MODIFIED) {
			if (pFile1->lastModified() != pFile2->lastModified())
				return _order==HTTP::SORT_ASC ? pFile1->lastModified() < pFile2->lastModified() : pFile2->lastModified() < pFile1->lastModified();
		}
		
		// NAME case
		if (pFile1->isFolder() && !pFile2->isFolder())
			return _order==HTTP::SORT_ASC;
		if (pFile2->isFolder() && !pFile1->isFolder())
			return _order==HTTP::SORT_DESC;
		int result = String::ICompare(pFile1->name(), pFile2->name());
		return _order==HTTP::SORT_ASC ? result<0 : result>0; 
	}
private:
	HTTP::SortField _field;
	HTTP::SortOrder _order;
};


bool HTTP::WriteDirectoryEntries(Exception& ex, BinaryWriter& writer, const string& serverAddress, const std::string& fullPath, const std::string& path, SortField sortField, SortOrder sortOrder) {

	vector<File*> files;
	FileSystem::ForEach forEach([&files](const string& path, UInt16 level){
		files.emplace_back(new File(path));
	});
	FileSystem::ListFiles(ex, fullPath, forEach);
	if (ex)
		return false;

	char sort[] = "D";
	if (sortOrder==SORT_DESC)
		sort[0] = 'A';

	// Write column names
	// Name		Modified	Size
	writer.write(EXPAND("<html><head><title>Index of "))
		.write(path).write(EXPAND("/</title><style>th {text-align: center;}</style></head><body><h1>Index of "))
		.write(path).write(EXPAND("/</h1><pre><table cellpadding=\"0\"><tr><th><a href=\"?N="))
		.write(sort).write(EXPAND("\">Name</a></th><th><a href=\"?M="))
		.write(sort).write(EXPAND("\">Modified</a></th><th><a href=\"?S="))
		.write(sort).write(EXPAND("\">Size</a></th></tr><tr><td colspan=\"3\"><hr></td></tr>"));

	// Write first entry - link to a parent directory
	if(!path.empty())
		writer.write(EXPAND("<tr><td><a href=\"http://"))
			.write(serverAddress).write(path)
			.write(EXPAND("/..\">Parent directory</a></td><td>&nbsp;-</td><td>&nbsp;&nbsp;-</td></tr>\n"));

	// Sort entries
	EntriesComparator comparator(sortField,sortOrder);
	std::sort(files.begin(), files.end(), comparator);

	// Write entries
	for (const File* pFile : files) {
		WriteDirectoryEntry(writer, serverAddress, path, *pFile);
		delete pFile;
	}

	// Write footer
	writer.write(EXPAND("</table></body></html>"));
	return true;
}

void HTTP::WriteDirectoryEntry(BinaryWriter& writer,const string& serverAddress,const string& path,const File& entry) {
	string size,date;
	if (entry.isFolder())
		size.assign("-");
	else if (entry.size()<1024)
		String::Format(size, entry.size());
	else if (entry.size() <	1048576)
		String::Format(size, Format<double>("%.1fk",entry.size()/1024.0));
	else if (entry.size() <	1073741824)
		String::Format(size, Format<double>("%.1fM",entry.size()/1048576.0));
	else
		String::Format(size, Format<double>("%.1fG",entry.size()/1073741824.0));

	writer.write(EXPAND("<tr><td><a href=\"http://")).write(serverAddress).write(path).write('/')
		.write(entry.name()).write(entry.isFolder() ? "/\">" : "\">")
		.write(entry.name()).write(entry.isFolder() ? "/" : "").write(EXPAND("</a></td><td>&nbsp;"))
		.write(Date(entry.lastModified()).toString("%d-%b-%Y %H:%M", date)).write(EXPAND("</td><td align=right>&nbsp;&nbsp;"))
		.write(size).write(EXPAND("</td></tr>\n"));
}

/// Cookie Writer class for writing Set-Cookie HTTP headers when client.properties(key, value, {expires=...}) is called
/// This implementation is based on RFC 6265 (but does not use 'Max-Age')
class SetCookieWriter : public DataWriter {
public:
	SetCookieWriter(Buffer& buffer, const HTTP::OnCookie& onCookie) : _onCookie(onCookie),_option(VALUE),_buffer(buffer) {
		String::Append(_buffer,"\r\nSet-Cookie: ");
	}

	UInt64	beginObject(const char* type=NULL) { return 0; }
	void	endObject() {}
	UInt64	beginArray(UInt32 size) { return 0; }
	void	endArray() {}

	void	writePropertyName(const char* value) {
		if (String::ICompare("domain", value) == 0)
			_option = DOM;
		else if (String::ICompare("path", value) == 0)
			_option = PATH;
		else if (String::ICompare("expires", value) == 0)
			_option = EXPIRES;
		else if (String::ICompare("secure", value) == 0)
			_option = SECURE;
		else if (String::ICompare("httponly", value) == 0)
			_option = HTTPONLY;
		else
			WARN("Unexpected option : ", value)
	}

	UInt64	writeDate(const Date& date) {
		if ((_option <= 2 || date) && writeHeader()) {
			string buffer;
			writeContent<String>(date.toString(Date::RFC1123_FORMAT, buffer));
		}
		_option = NO;
		return 0;
	}
	void	writeBoolean(bool value) {
		if ((_option <= 2 || value) && writeHeader())
			writeContent<String>(value ? "true" : "false");
		_option = NO;
	}
	void	writeNull() {
		if (_option <= 2 && writeHeader())
			writeContent<String>("null");
		_option = NO;
	}
	UInt64	writeBytes(const UInt8* data, UInt32 size) { writeString((const char*)data, size); return 0; }
	void	writeString(const char* value, UInt32 size) {
		if (_key.empty() && size && _option==VALUE) {
			_key.assign(value, size);
			String::Append(_buffer, _key, "=");
		}
		else if ((_option <= 2 || (String::ICompare(value, "false", size) != 0 && String::ICompare(value, "no", size) != 0 && String::ICompare(value, "0", size) != 0 && String::ICompare(value, "off", size) != 0 && String::ICompare(value, "null", size) != 0))&& writeHeader()) {
			writeContent<Buffer>(value, size);
			_option = NO;
		}
	}
	void	writeNumber(double value) {
		if ((_option <= 2 || value) && writeHeader()) {
			if (_option == EXPIRES) {
				string date;
				writeContent<String>(Date(Time::Now()+(Int64)(value*1000),Date::GMT).toString(Date::RFC1123_FORMAT, date));
			} else
				writeContent<String>(value);
		}
		_option = NO;
	}

private:

	bool writeHeader() {
		static const vector<const char*> Patterns({ "Domain=", "Path=", "Expires=", "Secure", "HttpOnly" });
		if (_option >= 0)
			String::Append(_buffer, "; ", Patterns[_option]);
		return _option<=2;
	}

	template <typename BufferClass, typename ...Args>
	void writeContent(Args&&... args) {
		UInt32 before = _buffer.size();
		BufferClass::Append(_buffer,args ...);
		if (_option == VALUE && _onCookie)
			_onCookie(_key.c_str(),STR _buffer.data()+before,_buffer.size()-before);
	}
	
	enum Option {
		NO = -2,
		VALUE=-1,
		DOM = 0,
		PATH = 1,
		EXPIRES = 2,
		SECURE = 3,
		HTTPONLY = 4
	};

	Buffer&				   _buffer;
	string				   _key;
	const HTTP::OnCookie&  _onCookie;
	Option				   _option;
};


bool HTTP::WriteSetCookie(DataReader& reader,Buffer& buffer, const OnCookie& onCookie) {

	if (reader.nextType() != DataReader::STRING)
		return false;
	UInt32 before = buffer.size();
	SetCookieWriter writer(buffer, onCookie);
	bool res = reader.read(writer)>1;
	if (!res)
		buffer.resize(before, true);
	return res;
}


} // namespace Mona
