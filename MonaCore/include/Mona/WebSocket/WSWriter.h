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
#include "Mona/JSONReader.h"
#include "Mona/TCPClient.h"
#include "Mona/WebSocket/WS.h"
#include "Mona/WebSocket/WSSender.h"

namespace Mona {


class WSWriter : public Writer, virtual Object {
public:

	WSWriter(TCPClient& client);
	
	UInt16			ping;

	State			state(State value=GET,bool minimal=false);
	void			flush(bool full=false);

	DataWriter&		writeInvocation(const std::string& name);
	DataWriter&		writeMessage();
	DataWriter&		writeResponse(UInt8 type);
	void			writeRaw(const UInt8* data, UInt32 size) { write(WS::TYPE_TEXT, data, size); }

	void			writePing() { write(WS::TYPE_PING, NULL, 0); }
	UInt16			elapsedSincePing();
	void			writePong(const UInt8* data, UInt32 size) { write(WS::TYPE_PONG, data, size); }
	void			close(int code = WS::CODE_NORMAL_CLOSE);

private:
	void			pack();
	void			createReader(PacketReader& reader, std::shared_ptr<DataReader>& pReader) { pReader.reset(new JSONReader(reader)); }
	void			createWriter(std::shared_ptr<DataWriter>& pWriter) { pWriter.reset(new JSONWriter(_client.poolBuffers())); }
	bool			hasToConvert(DataReader& reader) { return dynamic_cast<JSONReader*>(&reader) == NULL; }
	bool			writeMedia(MediaType type,UInt32 time,PacketReader& data);

	void			write(UInt8 type,const UInt8* data,UInt32 size);

	JSONWriter&		newDataWriter(bool modeRaw=false);

	UInt32									_sent;
	TCPClient&								_client;
	std::vector<std::shared_ptr<WSSender>>	_senders;
};



} // namespace Mona
