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
#include "Mona/PoolThreads.h"
#include "Mona/JSONReader.h"
#include "Mona/WebSocket/WS.h"
#include "Mona/WebSocket/WSSender.h"

namespace Mona {

#define WS_NORMAL_CLOSE				1000
#define WS_ENDPOINT_GOING_AWAY		1001
#define WS_PROTOCOL_ERROR			1002
#define WS_PAYLOAD_NOT_ACCEPTABLE	1003
#define WS_RESERVED					1004
#define WS_RESERVED_NO_STATUS_CODE	1005
#define WS_RESERVED_ABNORMAL_CLOSE	1006
#define WS_MALFORMED_PAYLOAD		1007
#define WS_POLICY_VIOLATION			1008
#define WS_PAYLOAD_TOO_BIG			1009
#define WS_EXTENSION_REQUIRED		1010
#define WS_UNEXPECTED_CONDITION		1011


class WSWriter : public Writer, virtual Object {
public:

	WSWriter(StreamSocket& socket);
	
	UInt16			ping;

	State			state(State value=GET,bool minimal=false);
	void			flush(bool full=false);

	DataWriter&		writeInvocation(const std::string& name);
	DataWriter&		writeMessage();
	void			writeRaw(const UInt8* data, UInt32 size) { write(WS_TEXT, data, size); }

	void			writePing() { write(WS_PING, NULL, 0); }
	UInt16			elapsedSincePing();
	void			writePong(const UInt8* data, UInt32 size) { write(WS_PONG, data, size); }
	void			close(int code);

private:
	void			pack();
	void			createReader(MemoryReader& reader, std::shared_ptr<DataReader>& pReader) { pReader.reset(new JSONReader(reader)); }
	void			createWriter(std::shared_ptr<DataWriter>& pWriter) { pWriter.reset(new JSONWriter()); }
	bool			hasToConvert(DataReader& reader) { return dynamic_cast<JSONReader*>(&reader) == NULL; }
	bool			writeMedia(MediaType type,UInt32 time,MemoryReader& data);

	void			write(UInt8 type,const UInt8* data,UInt32 size);

	JSONWriter&		newWriter();

	UInt32									_sent;
	StreamSocket&							_socket;
	std::list<std::shared_ptr<WSSender>>	_senders;
};



} // namespace Mona
