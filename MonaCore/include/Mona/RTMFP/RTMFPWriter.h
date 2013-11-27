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
#include "Mona/FlashWriter.h"
#include "Mona/MemoryReader.h"
#include "Mona/Trigger.h"
#include "Mona/AMFReader.h"
#include "Mona/Logs.h"
#include "Mona/RTMFP/BandWriter.h"
#include "Mona/RTMFP/RTMFPMessage.h"


#define MESSAGE_HEADER			0x80
#define MESSAGE_WITH_AFTERPART  0x10 
#define MESSAGE_WITH_BEFOREPART	0x20
#define MESSAGE_ABANDONMENT		0x02
#define MESSAGE_END				0x01

namespace Mona {

class Invoker;
class RTMFPWriter : public FlashWriter, virtual Object {
public:
	RTMFPWriter(const std::string& signature, BandWriter& band, WriterHandler* pHandler = NULL);
	RTMFPWriter(const std::string& signature,BandWriter& band,std::shared_ptr<RTMFPWriter>& pThis,WriterHandler* pHandler=NULL);
	virtual ~RTMFPWriter();

	const UInt64		id;
	const bool			critical;
	const UInt64		flowId;
	const std::string	signature;

	virtual Writer&		newWriter(WriterHandler* pHandler = NULL) { return *(new RTMFPWriter(signature, _band, pHandler)); }

	void				flush(bool full=false);

	void				acknowledgment(MemoryReader& reader);
	void				manage(Exception& ex, Invoker& invoker);

	template <typename ...Args>
	void fail(Args&&... args) {
		if (state() == CLOSED)
			return;
		WARN("RTMFPWriter ", id, " has failed, ", args ...);
		clear();
		_stage = _stageAck = _lostCount = _ackCount = 0;
        std::shared_ptr<RTMFPWriter> pWriter = _band.changeWriter(*new RTMFPWriter(*this));
        _band.initWriter(pWriter);
		_qos.reset();
		_reseted = true;
	}

	void				clear();
	void				close(int code=0);
	bool				consumed() { return _messages.empty() && state() == CLOSED; }

	UInt64				stage() { return _stage; }

	State				state(State value=GET,bool minimal=false);

	bool				writeMedia(MediaType type,UInt32 time,MemoryReader& data);
	void				writeRaw(const UInt8* data,UInt32 size);
	void				writeMember(const Peer& peer);

private:
	RTMFPWriter(RTMFPWriter& writer);
	
	UInt32					headerSize(UInt64 stage);
	void					flush(MemoryWriter& writer,UInt64 stage,UInt8 flags,bool header,BinaryReader& reader,UInt16 size);

	void					raiseMessage();
	RTMFPMessageBuffered&	createBufferedMessage();
	AMFWriter&				write(AMF::ContentType type,UInt32 time=0,MemoryReader* pData=NULL);

	void					createReader(MemoryReader& reader, std::shared_ptr<DataReader>& pReader) { pReader.reset(new AMFReader(reader)); }
	void					createWriter(std::shared_ptr<DataWriter>& pWriter) { pWriter.reset(new AMFWriter());pWriter->stream.next(6); }
	bool					hasToConvert(DataReader& reader) { return dynamic_cast<AMFReader*>(&reader) == NULL; }

	Trigger						_trigger;

	int			 				_connectedSize;
	std::list<RTMFPMessage*>	_messages;
	UInt64						_stage;
	std::list<RTMFPMessage*>	_messagesSent;
	UInt64						_stageAck;
	UInt32						_lostCount;
	UInt32						_ackCount;
	UInt32						_repeatable;
	BandWriter&					_band;

	UInt32						_boundCount;
	bool						_reseted;

	static RTMFPMessageNull		_MessageNull;
};


} // namespace Mona
