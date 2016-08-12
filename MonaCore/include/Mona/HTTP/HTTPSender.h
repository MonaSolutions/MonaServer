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
#include "Mona/File.h"
#include "Mona/Client.h"
#include "Mona/DataReader.h"
#include "Mona/HTTP/HTTP.h"
#include "Mona/HTTP/HTTPPacket.h"



namespace Mona {


class HTTPSender : public TCPSender, public virtual Object {
public:
	HTTPSender(const SocketAddress& address,HTTPPacket& request,const PoolBuffers& poolBuffers,const std::string& relativePath, PoolBuffer& pSetCookieBuffer);

	bool			newHeaders() const { return _newHeaders; }

	// if data==NULL and size==1 means "live stream" (no content-length), if data==NULL and size>1 it will use a StringWriter (raw serialization)
	DataWriter&		write(const char* code, HTTP::ContentType type = HTTP::CONTENT_TEXT, const char* subType = "html", const UInt8* data=NULL,UInt32 size=0);
	DataWriter&		writeResponse(const char* code="200 OK",bool rawWithoutLength=false);
	void			writeFile(const std::string& path, const std::shared_ptr<Parameters>& pParameters);
	BinaryWriter&	writeRaw();

	void writeError(const std::string& error,int code) {
		writeError(code, error);
		_connection = HTTP::CONNECTION_CLOSE;
	}

	const UInt8*	data() const { return _pWriter ? _pWriter->packet.data() : NULL; }
	UInt32			size() const { return _pWriter ? _pWriter->packet.size() : 0; }

private:
	template <typename ...Args>
	void writeError(int code, Args&&... args) {
		std::string title;
		PacketWriter& writer(write(String::Format(title, code, " ", HTTP::CodeToMessage(code)).c_str()).packet);
		HTML_BEGIN_COMMON_RESPONSE(writer, title)
			UInt32 size(writer.size());
			String::Append(writer,args ...);
			if (size == writer.size()) // nothing has been written
				writer.write(title);
		HTML_END_COMMON_RESPONSE(_serverAddress)
	}

	bool			run(Exception& ex);

	/// \brief  Write content file and replace the "<% key %>" field 
	/// by relating parameters[key]
	void			replaceTemplateTags(PacketWriter& packet, std::ifstream& ifile, const Parameters& parameters);

	/*! SocketSender override to disconnect socket if _connection == HTTP::CONNECTION_CLOSE */
	void			onSent(Socket& socket);

	// just for next writeResponse operation
	const HTTPPacket&					_request;

	PoolBuffer							_pSetCookieBuffer;
	const PoolBuffers&					_poolBuffers;
	File								_file;
	std::shared_ptr<Parameters>			_pFileParams;
	const std::string					_appPath; // Relative path of the application
	UInt8								_connection;
	HTTP::CommandType					_command;
	Date								_ifModifiedSince;
	std::string							_serverAddress;
	std::string							_origin;
	UInt32								_sizePos;
	std::unique_ptr<DataWriter>			_pWriter;
	std::string							_buffer;
	SocketAddress						_address;
	bool								_newHeaders;
};


} // namespace Mona
