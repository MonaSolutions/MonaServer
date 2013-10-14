/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
#include "Poco/Net/StreamSocket.h"

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


class WSWriter : public Writer {
public:

	WSWriter(SocketHandler<Poco::Net::StreamSocket>& handler);
	virtual ~WSWriter();

	Mona::UInt16	ping;

	State			state(State value=GET,bool minimal=false);
	void			flush(bool full=false);

	DataWriter&		writeInvocation(const std::string& name);
	DataWriter&		writeMessage();
	void			writeRaw(const Mona::UInt8* data,Mona::UInt32 size);

	void			writePing();
	Mona::UInt16	elapsedSincePing();
	void			writePong(const Mona::UInt8* data,Mona::UInt32 size);
	void			close(int code);

private:
	void			pack();
	void			createReader(MemoryReader& reader,Poco::SharedPtr<DataReader>& pReader);
	void			createWriter(Poco::SharedPtr<DataWriter>& pWriter);
	bool			hasToConvert(DataReader& reader);
	bool			writeMedia(MediaType type,Mona::UInt32 time,MemoryReader& data);

	void			write(Mona::UInt8 type,const Mona::UInt8* data,Mona::UInt32 size);

	JSONWriter&		newWriter();

	Mona::UInt32							_sent;
	SocketHandler<Poco::Net::StreamSocket>&	_handler;
	std::list<Poco::AutoPtr<WSSender>>		_senders;
};

inline bool WSWriter::hasToConvert(DataReader& reader) {
	return dynamic_cast<JSONReader*>(&reader)==NULL;
}

inline void WSWriter::createReader(MemoryReader& reader,Poco::SharedPtr<DataReader>& pReader) {
	pReader = new JSONReader(reader);
}

inline void WSWriter::createWriter(Poco::SharedPtr<DataWriter>& pWriter) {
	pWriter = new JSONWriter();
}

inline void WSWriter::writeRaw(const Mona::UInt8* data,Mona::UInt32 size) {
	write(WS_TEXT,data,size);
}

inline void WSWriter::writePing() {
	write(WS_PING,NULL,0);
}

inline void WSWriter::writePong(const Mona::UInt8* data,Mona::UInt32 size) {
	write(WS_PONG,data,size);
}

inline void WSWriter::close(int type) {
	write(WS_CLOSE,NULL,(Mona::UInt32)type);
	Writer::close(type);
}



} // namespace Mona
