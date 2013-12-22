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
#include "Mona/HTTP/HTTPSender.h"
#include "Mona/TCPClient.h"
#include "Mona/MediaContainer.h"

namespace Mona {



class HTTPWriter : public Writer, virtual Object {
public:
	enum DataType {
		HTML = 0,
		RAW,
		XML,
		SOAP,
		JSON,
		SVG,
		CSS
	};

	HTTPWriter(TCPClient& socket);

	std::shared_ptr<HTTPPacket>		pRequest;
	Time							timeout;

	State			state(State value=GET,bool minimal=false);
	void			flush(bool full=false);

	DataWriter&		writeInvocation(const std::string& type) { return write("200  OK", HTTP::ParseContentType(type.c_str(), _buffer), _buffer);}
	DataWriter&		writeMessage() { return writeResponse(HTML); }
	DataWriter&		writeResponse(UInt8 type);
	void			writeRaw(const UInt8* data, UInt32 size) { write("200 OK", HTTP::CONTENT_TEXT,"plain; charset=utf-8",data,size); }
	void			close(int code=0);

	DataWriter&		write(const std::string& code, HTTP::ContentType type=HTTP::CONTENT_TEXT, const std::string& subType="html; charset=utf-8",const UInt8* data=NULL,UInt32 size=0);
	void			writeFile(const FilePath& file) { return createSender().writeFile(file); }
	void			close(const Exception& ex);
private:
	bool			writeMedia(MediaType type,UInt32 time,MemoryReader& data);
	
	HTTPSender& createSender() {
		_senders.emplace_back(new HTTPSender(_socket.address(),pRequest));
		return *_senders.back();
	}

	TCPClient&								_socket;
	PoolThread*								_pThread;
	std::deque<std::shared_ptr<HTTPSender>>	_senders;
	std::deque<std::shared_ptr<HTTPSender>>	_sent;
	bool									_isMain;
	std::string								_buffer;

	// multimedia
	MediaContainer::Type					_mediaType;
	bool									_initMedia;
};



} // namespace Mona
