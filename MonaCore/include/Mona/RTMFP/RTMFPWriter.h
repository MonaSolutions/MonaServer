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
#include "Mona/RTMFP/RTMFP.h"
#include "Mona/Trigger.h"
#include "Mona/RTMFP/BandWriter.h"
#include "Mona/RTMFP/RTMFPMessage.h"
#include "Mona/FlashWriter.h"
#include "Mona/Logs.h"


#define MESSAGE_HEADER			0x80
#define MESSAGE_WITH_AFTERPART  0x10 
#define MESSAGE_WITH_BEFOREPART	0x20
#define MESSAGE_ABANDONMENT		0x02
#define MESSAGE_END				0x01

namespace Mona {

class Invoker;
class RTMFPWriter : public FlashWriter, public virtual Object {
public:
	RTMFPWriter(State state,const std::string& signature, BandWriter& band);
	RTMFPWriter(State state,const std::string& signature,BandWriter& band,std::shared_ptr<RTMFPWriter>& pThis);
	virtual ~RTMFPWriter();

	const UInt64		id;
	const bool			critical;
	const UInt64		flowId;
	const std::string	signature;

	virtual Writer&		newWriter() { return *(new RTMFPWriter(state(),signature, _band)); }

	bool				flush() { return flush(true); }

	void				acknowledgment(PacketReader& packet);
	void				manage(Exception& ex, Invoker& invoker);

	template <typename ...Args>
	void fail(Args&&... args) {
		if (state() == CLOSED)
			return;
		WARN("RTMFPWriter ", id, " has failed, ", args ...);
		abort();
		_stage = _stageAck = _lostCount = 0;
		 _ackCount = 0;
        std::shared_ptr<RTMFPWriter> pThis = _band.changeWriter(*new RTMFPWriter(*this));
        _band.initWriter(pThis);
		_qos.reset();
		_resetStream = true;
	}

	void				clear();
	void				abort();
	void				close(Int32 code=0);
	bool				consumed() { return _messages.empty() && state() == CLOSED; }

	UInt64				stage() { return _stage; }

	bool				writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties);
	void				writeRaw(const UInt8* data,UInt32 size);
	bool				writeMember(const Client& client);

private:
	RTMFPWriter(RTMFPWriter& writer);
	
	UInt32					headerSize(UInt64 stage);
	void					flush(BinaryWriter& writer,UInt64 stage,UInt8 flags,bool header,const RTMFPMessage& message, UInt32 offset, UInt16 size);
	bool					flush(bool full);

	void					raiseMessage();
	RTMFPMessageBuffered&	createMessage();
	AMFWriter&				write(AMF::ContentType type,UInt32 time=0,const UInt8* data=NULL, UInt32 size=0);

	Trigger						_trigger;

	std::deque<RTMFPMessage*>	_messages;
	UInt64						_stage;
	std::deque<RTMFPMessage*>	_messagesSent;
	UInt64						_stageAck;
	UInt32						_lostCount;
	double						_ackCount;
	UInt32						_repeatable;
	BandWriter&					_band;
	bool						_resetStream;

};


} // namespace Mona
