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
#include "Mona/Client.h"
#include "Mona/DataReader.h"
#include "Mona/RTSP/RTSP.h"
#include "Mona/RTSP/RTSPPacket.h"
#include "Mona/HTTP/HTTP.h"



namespace Mona {


class RTSPSender : public TCPSender, public virtual Object {
public:
	RTSPSender(const SocketAddress& address,RTSPPacket& request,const PoolBuffers& poolBuffers,const std::string& relativePath);

	bool			newHeaders() const { return _newHeaders; }

	// if data==NULL and size==1 means "live stream" (no content-length), if data==NULL and size>1 it will use a StringWriter (raw serialization)
	DataWriter&		write(const char* code, HTTP::ContentType type = HTTP::CONTENT_ABSENT, const char* subType = NULL, const UInt8* data=NULL,UInt32 size=0);
	DataWriter&		writeResponse(const char* code="200 OK",bool rawWithoutLength=false);
	BinaryWriter&	writeRaw();

	void writeError(const std::string& error,int code) {
		writeError(code, error);
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

	/*! SocketSender override to disconnect socket if _connection == HTTP::CONNECTION_CLOSE */
	void			onSent(Socket& socket);

	// just for next writeResponse operation
	const RTSPPacket&					_request;

	const PoolBuffers&					_poolBuffers;
	const std::string					_appPath; // Relative path of the application
	RTSP::CommandType					_command;
	std::string							_serverAddress;
	UInt32								_sizePos;
	std::unique_ptr<DataWriter>			_pWriter;
	std::string							_buffer;
	SocketAddress						_address;
	bool								_newHeaders;
};


} // namespace Mona
