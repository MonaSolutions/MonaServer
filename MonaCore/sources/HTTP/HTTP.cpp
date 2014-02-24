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

#include "Mona/XMLWriter.h"
#include "Mona/SOAPWriter.h"
#include "Mona/JSONWriter.h"
#include "Mona/CSSWriter.h"
#include "Mona/HTMLWriter.h"
#include "Mona/SVGWriter.h"
#include "Mona/RawWriter.h"

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

const UInt32 HTTP::DefaultTimeout(7000); // 7 sec

HTTP::ContentType HTTP::ExtensionToMIMEType(const string& extension, string& subType) {
	if (String::ICompare(extension, "js") == 0) {
		subType = "javascript";
		return CONTENT_APPLICATON;
	}
	if (String::ICompare(extension, "flv") == 0) {
		subType = "x-flv";
		return CONTENT_VIDEO;
	}
	if (String::ICompare(extension, "ts") == 0) {
		subType = "mpeg";
		return CONTENT_VIDEO;
	}
	if (String::ICompare(extension, "svg") == 0) {
		subType = "svg+xml";
		return CONTENT_IMAGE;
	}
	if (String::ICompare(extension, EXPAND_SIZE("m3u")) == 0) {
		subType = (extension.size() > 3 && extension[3] == '8') ? "x-mpegurl; charset=utf-8" : "x-mpegurl";
		return CONTENT_AUDIO;
	}
	 // TODO others
	subType = extension.empty() ? "plain" : extension;
	return CONTENT_TEXT;
}


string& HTTP::FormatContentType(ContentType type, const string& subType, string& value) {
	switch (type) {
		case CONTENT_TEXT:
			value.assign("text/");
			break;
		case CONTENT_IMAGE:
			value.assign("image/");
			break;
		case CONTENT_APPLICATON:
			value.assign("application/");
			break;
		case CONTENT_MULTIPART:
			value.assign("multipart/");
			break;
		case CONTENT_AUDIO:
			value.assign("audio/");
			break;
		case CONTENT_VIDEO:
			value.assign("video/");
			break;
		case CONTENT_MESSAGE:
			value.assign("message/");
			break;
		case CONTENT_MODEL:
			value.assign("model/");
			break;
		case CONTENT_EXAMPLE:
			value.assign("example/");
			break;
	}
	return value.append(subType);
}

DataWriter* HTTP::NewDataWriter(const PoolBuffers& poolBuffers,const string& subType) {
	if (String::ICompare(subType,EXPAND_SIZE("html"))==0 || String::ICompare(subType,EXPAND_SIZE("xhtml+xml"))==0)
		return new HTMLWriter(poolBuffers);
	if (String::ICompare(subType,EXPAND_SIZE("xml"))==0)
		return new XMLWriter(poolBuffers);
	if (String::ICompare(subType,EXPAND_SIZE("soap+xml"))==0)
		return new SOAPWriter(poolBuffers);
	if (String::ICompare(subType,EXPAND_SIZE("json"))==0)
		return new JSONWriter(poolBuffers);
	if (String::ICompare(subType,EXPAND_SIZE("svg+xml"))==0)
		return new SVGWriter(poolBuffers);
	if (String::ICompare(subType,EXPAND_SIZE("css"))==0)
		return new CSSWriter(poolBuffers);
	return new RawWriter(poolBuffers);
}

string& HTTP::CodeToMessage(UInt16 code, string& message) {
	auto found = CodeMessages.find(code);
	if (found != CodeMessages.end())
		message.assign(found->second);
	return message;
}

HTTP::CommandType HTTP::ParseCommand(Exception& ex,const char* value) {
	if (String::ICompare(value,EXPAND_SIZE("GET"))==0)
		return COMMAND_GET;
	if (String::ICompare(value,EXPAND_SIZE("PUSH"))==0)
		return COMMAND_PUSH;
	if (String::ICompare(value,EXPAND_SIZE("HEAD"))==0)
		return COMMAND_HEAD;
	if (String::ICompare(value,EXPAND_SIZE("OPTIONS"))==0)
		return COMMAND_OPTIONS;
	if (String::ICompare(value,EXPAND_SIZE("POST"))==0)
		return COMMAND_POST;
	ex.set(Exception::PROTOCOL, "Unknown HTTP command ", string(value, 4));
	return COMMAND_UNKNOWN;
}

UInt8 HTTP::ParseConnection(Exception& ex,const char* value) {
	vector<string> fields;
	UInt8 type(CONNECTION_ABSENT);
	for (const string& field : String::Split(value, ",", fields, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM)) {
		if (String::ICompare(field, "upgrade") == 0)
			type |= CONNECTION_UPGRADE;
		else if (String::ICompare(field, "keep-alive") == 0)
			type |= CONNECTION_KEEPALIVE;
		else if (String::ICompare(field, "close") == 0)
			type |= CONNECTION_CLOSE;
		else
			ex.set(Exception::PROTOCOL, "Unknown HTTP type connection ", field);
	}
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
	if (String::ICompare(value,EXPAND_SIZE("text")==0))
		return CONTENT_TEXT;
	if (String::ICompare(value,EXPAND_SIZE("image")==0))
		return CONTENT_IMAGE;
	if (String::ICompare(value,EXPAND_SIZE("application")==0))
		return CONTENT_APPLICATON;
	if (String::ICompare(value,EXPAND_SIZE("multipart")==0))
		return CONTENT_MULTIPART;
	if (String::ICompare(value,EXPAND_SIZE("audio")==0))
		return CONTENT_AUDIO;
	if (String::ICompare(value,EXPAND_SIZE("video")==0))
		return CONTENT_VIDEO;
	if (String::ICompare(value,EXPAND_SIZE("message")==0))
		return CONTENT_MESSAGE;
	if (String::ICompare(value,EXPAND_SIZE("model")==0))
		return CONTENT_MODEL;
	if (String::ICompare(value,EXPAND_SIZE("example")==0))
		return CONTENT_EXAMPLE;
	return CONTENT_TEXT; // default value
}




class EntriesComparator {
public:
	EntriesComparator(UInt8 options) : _options(options) {}

	bool operator() (const FilePath* pFile1,const FilePath* pFile2) {
		if (_options&HTTP::SORT_BY_SIZE)
			return _options&HTTP::SORT_ASC ? pFile1->size() < pFile2->size() : pFile2->size() < pFile1->size();
		if (_options&HTTP::SORT_BY_MODIFIED)
			return _options&HTTP::SORT_ASC ? pFile1->lastModified() < pFile2->lastModified() : pFile2->lastModified() < pFile1->lastModified();
		
		// NAME case
		if (pFile1->isDirectory() && !pFile2->isDirectory())
			return _options&HTTP::SORT_ASC;
		if (pFile2->isDirectory() && !pFile1->isDirectory())
			return _options&HTTP::SORT_DESC;
		int result = String::ICompare(pFile1->name(), pFile2->name());
		return _options&HTTP::SORT_ASC ? result<0 : result>0; 
	}
private:
	UInt8 _options;
};


void HTTP::WriteDirectoryEntries(BinaryWriter& writer, const string& serverAddress, const std::string& path, const Files& entries,UInt8 sortOptions) {

	char sort[] = "D";
	if (sortOptions&SORT_DESC)
		sort[0] = 'A';

	// Write column names
	// Name		Modified	Size
	writer.writeRaw("<html><head><title>Index of ",
		path, "/</title><style>th {text-align: center;}</style></head><body><h1>Index of ",
		path, "/</h1><pre><table cellpadding=\"0\"><tr><th><a href=\"?N=",
		sort, "\">Name</a></th><th><a href=\"?M=",
		sort, "\">Modified</a></th><th><a href=\"?S=",
		sort, "\">Size</a></th></tr><tr><td colspan=\"3\"><hr></td></tr>");

	// Write first entry - link to a parent directory
	if(!path.empty())
		writer.writeRaw("<tr><td><a href=\"http://", serverAddress,path,"/..\">Parent directory</a></td><td>&nbsp;-</td><td>&nbsp;&nbsp;-</td></tr>\n");

	// Sort entries
	vector<FilePath*> files(entries.count());
	int i = 0;
	for (const string& entry : entries)
		files[i++] = new FilePath(entry);
	EntriesComparator comparator(sortOptions);
	std::sort(files.begin(), files.end(),comparator);

	// Write entries
	for (const FilePath* pFile : files) {
		WriteDirectoryEntry(writer,serverAddress,path, *pFile);
		delete pFile;
	}

	// Write footer
	writer.writeRaw("</table></body></html>");
}

void HTTP::WriteDirectoryEntry(BinaryWriter& writer,const string& serverAddress,const string& path,const FilePath& entry) {
	string size,date;
	if (entry.isDirectory())
		size.assign("-");
	else if (entry.size()<1024)
		String::Format(size, entry.size());
	else if (entry.size() <	1048576)
		String::Format(size, Format<double>("%.1fk",entry.size()/1024.0));
	else if (entry.size() <	1073741824)
		String::Format(size, Format<double>("%.1fM",entry.size()/1048576.0));
	else
		String::Format(size, Format<double>("%.1fG",entry.size()/1073741824.0));

	writer.writeRaw("<tr><td><a href=\"http://", serverAddress, path, "/",
		entry.name(), entry.isDirectory() ? "/\">" : "\">",
		entry.name(), entry.isDirectory() ? "/" : "", "</a></td><td>&nbsp;",
		Date(entry.lastModified()).toString("%d-%b-%Y %H:%M", date), "</td><td align=right>&nbsp;&nbsp;",
		size, "</td></tr>\n");
}


} // namespace Mona
