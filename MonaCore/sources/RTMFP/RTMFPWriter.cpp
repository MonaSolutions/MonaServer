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

#include "Mona/RTMFP/RTMFPWriter.h"
#include "Mona/Peer.h"
#include "Mona/Util.h"



using namespace std;


namespace Mona {

RTMFPWriter::RTMFPWriter(State state,const string& signature, BandWriter& band, shared_ptr<RTMFPWriter>& pThis) : _resetStream(true), FlashWriter(state,band.poolBuffers()), id(0), _band(band), critical(false), _stage(0), _stageAck(0),  flowId(0), signature(signature), _repeatable(0), _lostCount(0), _ackCount(0) {
	pThis.reset(this);
	_band.initWriter(pThis);
	if (signature.empty())
		open();
}

RTMFPWriter::RTMFPWriter(State state,const string& signature, BandWriter& band) : _resetStream(true), FlashWriter(state,band.poolBuffers()), id(0), _band(band), critical(false), _stage(0), _stageAck(0), flowId(0), signature(signature), _repeatable(0), _lostCount(0), _ackCount(0) {
	shared_ptr<RTMFPWriter> pThis(this);
	_band.initWriter(pThis);
	if (signature.empty())
		open();
}

RTMFPWriter::RTMFPWriter(RTMFPWriter& writer) : _resetStream(true), FlashWriter(writer),_band(writer._band),
		critical(false),_repeatable(writer._repeatable), _stage(writer._stage),_stageAck(writer._stageAck),id(writer.id),
		_ackCount(writer._ackCount),_lostCount(writer._lostCount), flowId(0),signature(writer.signature) {
	reliable = true;
	close();
}

RTMFPWriter::~RTMFPWriter() {
	Writer::close();
	abort();
	if(!signature.empty())
		DEBUG("RTMFPWriter ",id," consumed");
}

void RTMFPWriter::abort() {
	// delete messages
	RTMFPMessage* pMessage;
	while(!_messages.empty()) {
		pMessage = _messages.front();
		_lostCount += pMessage->fragments.size();
		delete pMessage;
		_messages.pop_front();
	}
	while(!_messagesSent.empty()) {
		pMessage = _messagesSent.front();
		_lostCount += pMessage->fragments.size();
		if(pMessage->repeatable)
			--_repeatable;
		delete pMessage;
		_messagesSent.pop_front();
	}
	if(_stage>0) {
		createMessage(); // Send a MESSAGE_ABANDONMENT just in the case where the receiver has been created
		flush(false);
		_trigger.stop();
	}
}

void RTMFPWriter::clear() {
	for (RTMFPMessage* pMessage : _messages)
		delete pMessage;
	_messages.clear();
	FlashWriter::clear();
}

void RTMFPWriter::close(Int32 code) {
	if(state()==CLOSED)
		return;
	if(_stage>0 || _messages.size()>0)
		createMessage(); // Send a MESSAGE_END just in the case where the receiver has been created (or will be created)
	Writer::close(code);
}

void RTMFPWriter::acknowledgment(PacketReader& packet) {

	UInt64 bufferSize = packet.read7BitLongValue(); // TODO use this value in reliability mechanism?
	
	if(bufferSize==0) {
		// In fact here, we should send a 0x18 message (with id flow),
		// but it can create a loop... We prefer the following behavior
		fail("Negative acknowledgment");
		return;
	}

	UInt64 stageAckPrec = _stageAck;
	UInt64 stageReaden = packet.read7BitLongValue();
	UInt64 stage = _stageAck+1;

	if(stageReaden>_stage) {
		ERROR("Acknowledgment received ",stageReaden," superior than the current sending stage ",_stage," on writer ",id);
		_stageAck = _stage;
	} else if(stageReaden<=_stageAck) {
		// already acked
		if(packet.available()==0)
			DEBUG("Acknowledgment ",stageReaden," obsolete on writer ",id);
	} else
		_stageAck = stageReaden;

	UInt64 maxStageRecv = stageReaden;
	UInt32 pos=packet.position();

	while(packet.available()>0)
		maxStageRecv += packet.read7BitLongValue()+packet.read7BitLongValue()+2;
	if(pos != packet.position()) {
		// TRACE(stageReaden,"..x"Util::FormatHex(reader.current(),reader.available()));
		packet.reset(pos);
	}

	UInt64 lostCount = 0;
	UInt64 lostStage = 0;
	bool repeated = false;
	bool header = true;
	bool stop=false;

	auto it=_messagesSent.begin();
	while(!stop && it!=_messagesSent.end()) {
		RTMFPMessage& message(**it);

		if(message.fragments.empty()) {
			CRITIC("RTMFPMessage ",(stage+1)," is bad formatted on fowWriter ",id);
			++it;
			continue;
		}

		map<UInt32,UInt64>::iterator itFrag=message.fragments.begin();
		while(message.fragments.end()!=itFrag) {
			
			// ACK
			if(_stageAck>=stage) {
				message.fragments.erase(message.fragments.begin());
				itFrag=message.fragments.begin();
				++_ackCount;
				++stage;
				continue;
			}

			// Read lost informations
			while(!stop) {
				if(lostCount==0) {
					if(packet.available()>0) {
						lostCount = packet.read7BitLongValue()+1;
						lostStage = stageReaden+1;
						stageReaden = lostStage+lostCount+packet.read7BitLongValue();
					} else {
						stop=true;
						break;
					}
				}
				// check the range
				if(lostStage>_stage) {
					// Not yet sent
					ERROR("Lost information received ",lostStage," have not been yet sent on writer ",id);
					stop=true;
				} else if(lostStage<=_stageAck) {
					// already acked
					--lostCount;
					++lostStage;
					continue;
				}
				break;
			}
			if(stop)
				break;
			
			// lostStage > 0 and lostCount > 0

			if(lostStage!=stage) {
				if(repeated) {
					++stage;
					++itFrag;
					header=true;
				} else // No repeated, it means that past lost packet was not repeatable, we can ack this intermediate received sequence
					_stageAck = stage;
				continue;
			}

			/// Repeat message asked!
			if(!message.repeatable) {
				if(repeated) {
					++itFrag;
					++stage;
					header=true;
				} else {
					INFO("RTMFPWriter ",id," : message ",stage," lost");
					--_ackCount;
					++_lostCount;
					_stageAck = stage;
				}
				--lostCount;
				++lostStage;
				continue;
			}

			repeated = true;
			// Don't repeate before that the receiver receives the itFrag->second sending stage
			if(itFrag->second >= maxStageRecv) {
				++stage;
				header=true;
				--lostCount;
				++lostStage;
				++itFrag;
				continue;
			}

			// Repeat message

			DEBUG("RTMFPWriter ",id," : stage ",stage," repeated");
			UInt32 fragment(itFrag->first);
			itFrag->second = _stage; // Save actual stage sending to wait that the receiver gets it before to retry
			UInt32 contentSize = message.size() - fragment; // available
			++itFrag;

			// Compute flags
			UInt8 flags = 0;
			if(fragment>0)
				flags |= MESSAGE_WITH_BEFOREPART; // fragmented
			if(itFrag!=message.fragments.end()) {
				flags |= MESSAGE_WITH_AFTERPART;
				contentSize = itFrag->first - fragment;
			}

			UInt32 size = contentSize+4;
			UInt32 availableToWrite(_band.availableToWrite());
			if(!header && size>availableToWrite) {
				_band.flush();
				header=true;
			}

			if(header)
				size+=headerSize(stage);

			if(size>availableToWrite)
				_band.flush();

			// Write packet
			size-=3;  // type + timestamp removed, before the "writeMessage"
			flush(_band.writeMessage(header ? 0x10 : 0x11,(UInt16)size)
				,stage,flags,header,message,fragment,contentSize);
			header=false;
			--lostCount;
			++lostStage;
			++stage;
		}

		if(message.fragments.empty()) {
			if(message.repeatable)
				--_repeatable;
			if(_ackCount || _lostCount) {
				_qos.add(_lostCount / (_lostCount + _ackCount));
				_ackCount=_lostCount=0;
			}
			
			delete *it;
			it=_messagesSent.erase(it);
		} else
			++it;
	
	}

	if(lostCount>0 && packet.available()>0)
		ERROR("Some lost information received have not been yet sent on writer ",id);


	// rest messages repeatable?
	if(_repeatable==0)
		_trigger.stop();
	else if(_stageAck>stageAckPrec || repeated)
		_trigger.reset();
}

void RTMFPWriter::manage(Exception& ex, Invoker& invoker) {
	if(!consumed() && !_band.failed()) {
		
		if(_trigger.raise(ex))
			raiseMessage();

		if (ex) {
			fail("RTMFPWriter can't deliver its data, ",ex.error());
			return;
		}
	}
	if(critical && state()==CLOSED) {
		ex.set(Exception::NETWORK, "Main flow writer closed, session is closing");
		return;
	}
	flush(false);
}

UInt32 RTMFPWriter::headerSize(UInt64 stage) { // max size header = 50
	UInt32 size= Util::Get7BitValueSize(id);
	size+= Util::Get7BitValueSize(stage);
	if(_stageAck>stage)
		CRITIC("stageAck ",_stageAck," superior to stage ",stage," on writer ",id);
	size+= Util::Get7BitValueSize(stage-_stageAck);
	size+= _stageAck>0 ? 0 : (signature.size()+(flowId==0?2:(4+Util::Get7BitValueSize(flowId))));
	return size;
}


void RTMFPWriter::flush(BinaryWriter& writer,UInt64 stage,UInt8 flags,bool header,const RTMFPMessage& message, UInt32 offset, UInt16 size) {
	if(_stageAck==0 && header)
		flags |= MESSAGE_HEADER;
	if(size==0)
		flags |= MESSAGE_ABANDONMENT;
	if(state()==CLOSED && _messages.size()==1) // On LAST message
		flags |= MESSAGE_END;

	// TRACE("RTMFPWriter ",id," stage ",stage);

	writer.write8(flags);

	if(header) {
		writer.write7BitLongValue(id);
		writer.write7BitLongValue(stage);
		writer.write7BitLongValue(stage-_stageAck);

		// signature
		if(_stageAck==0) {
			writer.write8((UInt8)signature.size()).write(signature);
			// No write this in the case where it's a new flow!
			if(flowId>0) {
				writer.write8(1+Util::Get7BitValueSize(flowId)); // following size
				writer.write8(0x0a); // Unknown!
				writer.write7BitLongValue(flowId);
			}
			writer.write8(0); // marker of end for this part
		}
	}

	if (size == 0)
		return;

	if (offset < message.frontSize()) {
		UInt8 count = message.frontSize()-offset;
		if (size<count)
			count = (UInt8)size;
		writer.write(message.front()+offset,count);
		size -= count;
		if (size == 0)
			return;
		offset += count;
	}

	writer.write(message.body()+offset-message.frontSize(), size);
}

void RTMFPWriter::raiseMessage() {
	bool header = true;
	bool stop = true;
	bool sent = false;
	UInt64 stage = _stageAck+1;

	for(RTMFPMessage* pMessage : _messagesSent) {
		RTMFPMessage& message(*pMessage);
		
		if(message.fragments.empty())
			break;

		// not repeat unbuffered messages
		if(!message.repeatable) {
			stage += message.fragments.size();
			header = true;
			continue;
		}
		
		/// HERE -> message repeatable AND already flushed one time!

		if(stop) {
			_band.flush(); // To repeat message, before we must send precedent waiting mesages
			stop = false;
		}

		map<UInt32,UInt64>::const_iterator itFrag=message.fragments.begin();
		UInt32 available = message.size()-itFrag->first;
	
		while(itFrag!=message.fragments.end()) {
			UInt32 contentSize = available;
			UInt32 fragment(itFrag->first);
			++itFrag;

			// Compute flags
			UInt8 flags = 0;
			if(fragment>0)
				flags |= MESSAGE_WITH_BEFOREPART; // fragmented
			if(itFrag!=message.fragments.end()) {
				flags |= MESSAGE_WITH_AFTERPART;
				contentSize = itFrag->first - fragment;
			}

			UInt32 size = contentSize+4;

			if(header)
				size+=headerSize(stage);

			// Actual sending packet is enough large? Here we send just one packet!
			if(size>_band.availableToWrite()) {
				if(!sent)
					ERROR("Raise messages on writer ",id," without sending!");
				DEBUG("Raise message on writer ",id," finishs on stage ",stage);
				return;
			}
			sent=true;

			// Write packet
			size-=3;  // type + timestamp removed, before the "writeMessage"
			flush(_band.writeMessage(header ? 0x10 : 0x11,(UInt16)size)
				,stage++,flags,header,message,fragment,contentSize);
			available -= contentSize;
			header=false;
		}
	}

	if(stop)
		_trigger.stop();
}

bool RTMFPWriter::flush(bool full) {

	if(_messagesSent.size()>100)
		TRACE("_messagesSent.size()=",_messagesSent.size());

	if(state()==OPENING) {
		ERROR("Violation policy, impossible to flush data on a opening writer");
		return false;
	}

	bool hasSent(false);

	// flush
	bool header = !_band.canWriteFollowing(*this);

	while(!_messages.empty()) {
		hasSent = true;

		RTMFPMessage& message(*_messages.front());

		if(message.repeatable) {
			++_repeatable;
			_trigger.start();
		}

		UInt32 fragments= 0;
		UInt32 available = message.size();
	
		do {

			++_stage;

			// Actual sending packet is enough large?
			UInt32 contentSize = _band.availableToWrite();
			UInt32 headerSize = (header && contentSize<62) ? this->headerSize(_stage) : 0; // calculate only if need!
			if(contentSize<(headerSize+12)) { // 12 to have a size minimum of fragmentation
				_band.flush(); // send packet (and without time echo)
				header=true;
			}

			contentSize = available;
			UInt32 size = contentSize+4;
			
			if(header)
				size+= headerSize>0 ? headerSize : this->headerSize(_stage);

			// Compute flags
			UInt8 flags = 0;
			if(fragments>0)
				flags |= MESSAGE_WITH_BEFOREPART;

			bool head = header;
			UInt32 availableToWrite(_band.availableToWrite());
			if(size>availableToWrite) {
				// the packet will change! The message will be fragmented.
				flags |= MESSAGE_WITH_AFTERPART;
				contentSize = availableToWrite-(size-contentSize);
				size=availableToWrite;
				header=true;
			} else
				header=false; // the packet stays the same!

			// Write packet
			size-=3; // type + timestamp removed, before the "writeMessage"
			flush(_band.writeMessage(head ? 0x10 : 0x11,(UInt16)size,this),_stage,flags,head,message,fragments,contentSize);

			
			message.fragments[fragments] = _stage;
			available -= contentSize;
			fragments += contentSize;

		} while(available>0);

		_qos.add(message.size(),_band.ping());
		_messagesSent.emplace_back(&message);
		_messages.pop_front();
	}

	if (full)
		_band.flush();
	return hasSent;
}

RTMFPMessageBuffered& RTMFPWriter::createMessage() {
	if (state() == CLOSED || signature.empty() || _band.failed()) {// signature.empty() means that we are on the writer of FlowNull
		static RTMFPMessageBuffered MessageNull;
		return MessageNull;
	}
	RTMFPMessageBuffered* pMessage = new RTMFPMessageBuffered(_band.poolBuffers(),reliable);
	_messages.emplace_back(pMessage);
	return *pMessage;
}

AMFWriter& RTMFPWriter::write(AMF::ContentType type,UInt32 time,const UInt8* data, UInt32 size) {
	if (type < AMF::AUDIO || type > AMF::VIDEO)
		time = 0; // Because it can "dropped" the packet otherwise (like if the Writer was not reliable!)
	if(data && !reliable && state()==OPENED && !_band.failed() && !signature.empty()) {
		_messages.emplace_back(new RTMFPMessageUnbuffered(type,time,data,size));
		flush(false);
        return AMFWriter::Null;
	}
	AMFWriter& amf = createMessage().writer();
	BinaryWriter& binary(amf.packet);
	binary.write8(type).write32(time);
	if(type==AMF::DATA_AMF3)
		binary.write8(0);
	if(data)
		binary.write(data,size);
	return amf;
}

bool RTMFPWriter::writeMember(const Client& client) {
	createMessage().writer().packet.write8(0x0b).write(client.id,ID_SIZE); // 0x0b unknown
	return true;
}

void RTMFPWriter::writeRaw(const UInt8* data,UInt32 size) {
	if(reliable || state()==OPENING) {
		createMessage().writer().packet.write(data,size);
		return;
	}
	if(state()==CLOSED || signature.empty() || _band.failed()) // signature.empty() means that we are on the writer of FlowNull
		return;
	_messages.emplace_back(new RTMFPMessageUnbuffered(data,size));
	flush(false);
}

bool RTMFPWriter::writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties) {
	if (type==INIT)
		_resetStream = false;
	return _resetStream ? false : FlashWriter::writeMedia(type,time,packet,properties);
}


} // namespace Mona
