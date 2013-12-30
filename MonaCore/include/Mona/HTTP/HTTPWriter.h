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
#include "Mona/HTTPPacketWriter.h"
#include "Mona/HTTP/HTTPSender.h"
#include "Mona/StreamSocket.h"

namespace Mona {

#define HLS_PACKET_SIZE					188

class HTTPWriter : public Writer, virtual Object {
public:

	enum TypeFrame {
		FIRST=0,
		OTHER,
		LAST,
		UNIQUE,
	};

	HTTPWriter(StreamSocket& socket);

	virtual State			state(State value=GET,bool minimal=false);
	virtual void			flush(bool full=false);

	virtual DataWriter&		writeInvocation(const std::string& name);
	virtual DataWriter&		writeMessage();
	virtual void			writeRaw(const UInt8* data, UInt32 size) { newWriter().writer.writeRaw(data, size); }
	virtual bool			writeMedia(MediaType type,UInt32 time,MemoryReader& data);

	// TODO ?void			close(int code);
	
private:
	bool				hasToConvert(DataReader& reader) { return dynamic_cast<HTTPPacketWriter*>(&reader) == NULL; }
	HTTPPacketWriter&	newWriter();
	
	void				writeHeader();
	void				writeVideoPacket(BinaryWriter& writer, UInt32 available, UInt32 time, UInt8* pData, bool isMetadata, TypeFrame type);

	StreamSocket&								_socket;
	std::list<std::shared_ptr<HTTPSender>>		_senders;
	static UInt32								CounterRow;
	static UInt32								CounterFrame;
	static char									CounterA;
	static UInt32								BeginTime;

	// TODO don't do it static
	static UInt8								HLSInitVideoBuff[];
	static UInt8								BeginBuff1[];
	static UInt8								BeginBuff2[];
};



} // namespace Mona
