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
#include "Mona/TCPSender.h"
#include "Mona/Writer.h"
#include "Mona/Path.h"
#include "Mona/HTTP/HTTP.h"
#include "Mona/HTTP/HTTPPacket.h"
#include "Mona/Client.h"


namespace Mona {


#define HTML_BEGIN_COMMON_RESPONSE(BINARYWRITER,TITLE) \
	BINARYWRITER.writeRaw("<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>", TITLE,"</title></head><body><h1>",TITLE, "</h1><p>");

#define HTML_END_COMMON_RESPONSE(BINARYWRITER,ADDRESS) \
	BINARYWRITER.writeRaw("</p><hr><address>Mona Server at ", ADDRESS ,"</address></body></html>\r\n");


#define HTTP_BEGIN_HEADER(BINARYWRITER)  BINARYWRITER.clear(BINARYWRITER.size()-2);
#define HTTP_ADD_HEADER(BINARYWRITER,NAME,VALUE) BINARYWRITER.writeRaw(NAME, ": ", VALUE , "\r\n");
#define HTTP_END_HEADER(BINARYWRITER)  BINARYWRITER.writeRaw("\r\n");


class HTTPSender : public TCPSender, public virtual Object {
public:
	HTTPSender(const SocketAddress& address,HTTPPacket& request,const PoolBuffers& poolBuffers,const std::string& relativePath);

	DataWriter&		writer(const std::string& code, HTTP::ContentType type, const std::string& subType,const UInt8* data,UInt32 size);
	void			writeFile(const Path& file, UInt8 sortOptions, bool isApp) { _file = file; _sortOptions = sortOptions; _isApp = isApp; }

	const UInt8*	data() { return _pWriter ? _pWriter->packet.data() : NULL; }
	UInt32			size() { return _pWriter ? _pWriter->packet.size() : 0; }

	BinaryWriter&	writeRaw(const PoolBuffers& poolBuffers);

	void writeError(const std::string& error,int code) {
		writeError(code, error);
		_connection = HTTP::CONNECTION_CLOSE;
	}

private:
	template <typename ...Args>
	void writeError(int code,Args&&... args) {
		std::string title;
		BinaryWriter& writer = write(String::Format(title, code, " ", HTTP::CodeToMessage(code))).packet;
		HTML_BEGIN_COMMON_RESPONSE(writer, title)
			UInt32 size(writer.size());
			writer.writeRaw(args ...);
			if (size == writer.size()) // nothing has been written
				writer.writeRaw(title);
		HTML_END_COMMON_RESPONSE(writer, _serverAddress)
	}

	bool			run(Exception& ex);

	DataWriter&		write(const std::string& code, HTTP::ContentType type = HTTP::CONTENT_TEXT, const std::string& subType = "html") { return writer(code, type, subType, NULL, 0); }

	/// \brief  Write content file and replace the "<% key %>" field 
	/// by relating parameters[key]
	void			replaceTemplateTags(PacketWriter& packet, std::ifstream& ifile, const Parameters& parameters, UInt32 sizeParameters);


	/*! SocketSender override to disconnect socket if _connection == HTTP::CONNECTION_CLOSE */
	void			onSent(Socket& socket);


	const PoolBuffers&						_poolBuffers;
	bool									_isApp;
	Path									_file;
	const std::string						_appPath; // Relative path of the application
	UInt8									_sortOptions;
	const std::shared_ptr<HTTPSendingInfos> _pInfos;
	UInt8									_connection;
	HTTP::CommandType						_command;
	Date									_ifModifiedSince;
	std::string								_serverAddress;
	UInt32									_sizePos;
	std::unique_ptr<DataWriter>				_pWriter;
	std::string								_buffer;
	SocketAddress							_address;
};


} // namespace Mona
