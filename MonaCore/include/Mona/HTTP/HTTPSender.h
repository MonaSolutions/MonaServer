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
#include "Mona/Writer.h"
#include "Mona/TCPSender.h"
#include "Mona/FilePath.h"
#include "Mona/HTTP/HTTP.h"
#include "Mona/HTTP/HTTPPacket.h"
#include "Mona/Client.h"


namespace Mona {


#define HTML_BEGIN_COMMON_RESPONSE(BINARYWRITER,TITLE) \
	BINARYWRITER.writeRaw("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>", TITLE,"</title></head><body><h1>",TITLE, "</h1><p>");

#define HTML_END_COMMON_RESPONSE(BINARYWRITER,ADDRESS) \
	BINARYWRITER.writeRaw("</p><hr><address>Mona Server at ", ADDRESS ,"</address></body></html>\r\n");


#define HTTP_BEGIN_HEADER(DATAWRITER)  DATAWRITER.stream.resetWriting(DATAWRITER.stream.size()-2);
#define HTTP_ADD_HEADER(BINARYWRITER,NAME,VALUE) BINARYWRITER.writeRaw(NAME, ": ", VALUE , "\r\n");
#define HTTP_END_HEADER(BINARYWRITER)  BINARYWRITER.writeRaw("\r\n");


class HTTPSender : public TCPSender, virtual Object {
public:
	HTTPSender(const SocketAddress& address,const std::shared_ptr<HTTPPacket>& pRequest);

	DataWriter&		writer(const std::string& code, HTTP::ContentType type, const std::string& subType,const UInt8* data,UInt32 size);
	void			writeError(int code, const std::string& description,bool close=false);
	void			writeFile(const FilePath& file) { _file = file; }

	const UInt8*	begin() { return _pWriter ? _pWriter->stream.data() : NULL; }
	UInt32			size() { return _pWriter ? _pWriter->stream.size() : 0; }

	BinaryWriter&	writeRaw();
private:
	bool			run(Exception& ex);

	DataWriter&		write(const std::string& code, HTTP::ContentType type = HTTP::CONTENT_TEXT, const std::string& subType = "html; charset=utf-8") { return writer(code, type, subType, NULL, 0); }

	FilePath						_file;
	std::shared_ptr<HTTPPacket>		_pRequest;
	UInt32							_sizePos;
	std::unique_ptr<DataWriter>		_pWriter;
	std::string						_buffer;
	SocketAddress					_address;
};


} // namespace Mona
