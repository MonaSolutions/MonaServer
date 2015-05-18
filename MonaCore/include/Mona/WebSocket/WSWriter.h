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
#include "Mona/TCPSession.h"
#include "Mona/MIME.h"
#include "Mona/WebSocket/WS.h"
#include "Mona/WebSocket/WSSender.h"

namespace Mona {


class WSWriter : public Writer, public virtual Object {
public:

	WSWriter(TCPSession& session);

	void			clear() { _senders.clear(); }
	bool			flush();

	DataWriter&		writeInvocation(const char* name);
	DataWriter&		writeMessage();
	DataWriter&		writeResponse(UInt8 type);
	void			writeRaw(const UInt8* data, UInt32 size) { write(WS::TYPE_BINARY, data, size); }

	void			writePing() { write(WS::TYPE_PING, NULL, 0); }
	void			writePong(const UInt8* data, UInt32 size) { write(WS::TYPE_PONG, data, size); }
	void			close(Int32 code);

private:
	void			pack();

	bool			writeMedia(MediaType type,UInt32 time,PacketReader& data,const Parameters& properties);

	void			write(UInt8 type,const UInt8* data,UInt32 size);

	DataWriter&		newDataWriter(bool modeRaw=false);

	TCPSession&								_session;
	std::vector<std::shared_ptr<WSSender>>	_senders;
	MIME::Type								_dataType;
};



} // namespace Mona
