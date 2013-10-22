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
#include "Mona/FlashWriter.h"
#include "Mona/MemoryReader.h"
#include "Mona/Trigger.h"
#include "Mona/AMFReader.h"
#include "Mona/RTMFP/BandWriter.h"
#include "Mona/RTMFP/RTMFPMessage.h"
#include "Poco/RefCountedObject.h"

#define MESSAGE_HEADER			0x80
#define MESSAGE_WITH_AFTERPART  0x10 
#define MESSAGE_WITH_BEFOREPART	0x20
#define MESSAGE_ABANDONMENT		0x02
#define MESSAGE_END				0x01

namespace Mona {

class Invoker;
class RTMFPWriter : public FlashWriter, public Poco::RefCountedObject {
public:
	RTMFPWriter(const std::string& signature,BandWriter& band,WriterHandler* pHandler=NULL);
	virtual ~RTMFPWriter();

	const Mona::UInt64		id;
	const bool				critical;
	const Mona::UInt64		flowId;
	const std::string		signature;

	virtual Writer&	newWriter(WriterHandler* pHandler=NULL);

	void			flush(Exception& ex, bool full=false);

	void			acknowledgment(MemoryReader& reader);
	void			manage(Exception& ex, Invoker& invoker);

	void			fail(const std::string& error);
	void			clear();
	void			close(int code=0);
	bool			consumed();

	Mona::UInt64	stage();

	State			state(State value=GET,bool minimal=false);

	bool			writeMedia(MediaType type,Mona::UInt32 time,MemoryReader& data);
	void			writeRaw(Exception& ex, const Mona::UInt8* data,Mona::UInt32 size);
	void			writeMember(const Peer& peer);

private:
	RTMFPWriter(RTMFPWriter& writer);
	
	Mona::UInt32			headerSize(Mona::UInt64 stage);
	void					flush(MemoryWriter& writer,Mona::UInt64 stage,Mona::UInt8 flags,bool header,BinaryReader& reader,Mona::UInt16 size);

	void					raiseMessage(Exception& ex);
	RTMFPMessageBuffered&	createBufferedMessage();
	AMFWriter&				write(AMF::ContentType type,Mona::UInt32 time=0,MemoryReader* pData=NULL);

	void					createReader(MemoryReader& reader,Poco::SharedPtr<DataReader>& pReader);
	void					createWriter(Poco::SharedPtr<DataWriter>& pWriter);
	bool					hasToConvert(DataReader& reader);

	Trigger					_trigger;

	int			 			_connectedSize;
	std::list<RTMFPMessage*>		_messages;
	Mona::UInt64			_stage;
	std::list<RTMFPMessage*>		_messagesSent;
	Mona::UInt64			_stageAck;
	Mona::UInt32			_lostCount;
	Mona::UInt32			_ackCount;
	Mona::UInt32			_repeatable;
	BandWriter&				_band;

	Mona::UInt32			_boundCount;
	bool					_reseted;

	static RTMFPMessageNull		_MessageNull;
};

inline Writer& RTMFPWriter::newWriter(WriterHandler* pHandler) {
	return *(new RTMFPWriter(signature,_band,pHandler));
}

inline void RTMFPWriter::createWriter(Poco::SharedPtr<DataWriter>& pWriter) {
	pWriter = new AMFWriter();
	pWriter->stream.next(6);
}

inline void RTMFPWriter::createReader(MemoryReader& reader,Poco::SharedPtr<DataReader>& pReader) {
	pReader = new AMFReader(reader);
}

inline bool RTMFPWriter::hasToConvert(DataReader& reader) {
	return dynamic_cast<AMFReader*>(&reader)==NULL;
}

inline Mona::UInt64 RTMFPWriter::stage() {
	return _stage;
}
inline bool RTMFPWriter::consumed() {
	return _messages.empty() && state()==CLOSED;
}

} // namespace Mona
