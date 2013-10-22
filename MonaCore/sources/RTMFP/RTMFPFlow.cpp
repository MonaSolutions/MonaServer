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

#include "Mona/RTMFP/RTMFPFlow.h"
#include "Mona/Invoker.h"
#include "Mona/FlashMainStream.h"
#include "Mona/Logs.h"
#include "Mona/Util.h"
#include "Poco/Format.h"
#include <cstring>

using namespace std;
using namespace Poco;

namespace Mona {


class RTMFPPacket {
public:
	RTMFPPacket(MemoryReader& fragment) : fragments(1),_pMessage(NULL),_buffer((string::size_type)fragment.available()) {
		if(_buffer.size()>0)
			memcpy(&_buffer[0],fragment.current(),_buffer.size());
	}
	~RTMFPPacket() {
		if(_pMessage)
			delete _pMessage;
	}

	void add(MemoryReader& fragment) {
		string::size_type old = _buffer.size();
		_buffer.resize(old + (string::size_type)fragment.available());
		if(_buffer.size()>old)
			memcpy(&_buffer[old],fragment.current(),(size_t)fragment.available());
		++(UInt16&)fragments;
	}

	MemoryReader* release() {
		if(_pMessage) {
			ERROR("RTMFPPacket already released!");
			return _pMessage;
		}
		_pMessage = new MemoryReader(_buffer.size()==0 ? NULL : &_buffer[0],_buffer.size());
		(UInt16&)_pMessage->fragments = fragments;
		return _pMessage;
	}

	const UInt32	fragments;
private:
	vector<UInt8>	_buffer;
	MemoryReader*  _pMessage;
};


class RTMFPFragment : public Buffer<UInt8> {
public:
	RTMFPFragment(MemoryReader& data,UInt8 flags) : flags(flags),Buffer<UInt8>(data.available()) {
		data.readRaw(begin(),size());
	}
	Mona::UInt8					flags;
};


RTMFPFlow::RTMFPFlow(UInt64 id,const string& signature,Peer& peer,Invoker& invoker,BandWriter& band) : _numberLostFragments(0),id(id),_stage(0),_completed(false),_pPacket(NULL),_pStream(NULL),_band(band),_pWriter(new RTMFPWriter(signature,band),true) {
	if(signature.empty())
		return;

	if(_pWriter->flowId==0)
		((UInt64&)_pWriter->flowId)=id;

	// create code prefix for a possible response
	// get flash stream process engine

	if(signature.size()>4 && signature.compare(0,5,"\x00\x54\x43\x04\x00",5)==0)
		(bool&)_pWriter->critical = true; // NetConnection
	else if(signature.size()>3 && signature.compare(0,4,"\x00\x54\x43\x04",4)==0)
		_pStream = invoker.getFlashStream(MemoryReader((const UInt8*)signature.c_str()+4,signature.length()-4).read7BitValue()); // NetStream
	else if(signature.size()>2 && signature.compare(0,3,"\x00\x47\x43",3)==0)
		(UInt64&)_pWriter->flowId = id; // NetGroup
	
	if(_pStream.isNull())
		_pStream = new FlashMainStream(invoker,peer);
}

RTMFPFlow::~RTMFPFlow() {
	complete();
	_pWriter->close();
}

void RTMFPFlow::complete() {
	if(_completed)
		return;

	if(!_pStream.isNull()) // FlowNull instance, not display the message in FullNull case
		DEBUG("RTMFPFlow ",id," consumed");

	// delete fragments
	map<UInt64,RTMFPFragment*>::const_iterator it;
	for(it=_fragments.begin();it!=_fragments.end();++it)
		delete it->second;
	_fragments.clear();

	// delete receive buffer
	if(_pPacket) {
		delete _pPacket;
		_pPacket=NULL;
	}

	_completed=true;
}

void RTMFPFlow::fail(const string& error) {
	ERROR("RTMFPFlow ",id," failed : ",error);
	if(!_completed) {
		Exception ex;
		BinaryWriter& writer = _band.writeMessage(ex, 0x5e,Util::Get7BitValueSize(id)+1);
		if (ex)
			ERROR("Error during fail : ", ex.error());
		writer.write7BitLongValue(id);
		writer.write8(0); // unknown
	}
}

AMF::ContentType RTMFPFlow::unpack(MemoryReader& reader) {
	if(reader.available()==0)
		return AMF::EMPTY;
	AMF::ContentType type = (AMF::ContentType)reader.read8();
	switch(type) {
		// amf content
		case 0x11:
			reader.next(1);
		case AMF::INVOCATION:
			reader.next(4);
			return AMF::INVOCATION;
		case AMF::DATA:
			reader.next(5);
		case AMF::AUDIO:
		case AMF::VIDEO:
			break;
		// raw data
		case AMF::RAW:
			reader.next(4);
		case 0x01:
			break;
		default:
			ERROR("Unpacking type '",Format<UInt8>("%02x",(UInt8)type),"' unknown");
			break;
	}
	return type;
}

void RTMFPFlow::commit() {

	// Lost informations!
	UInt32 size = 0;
	list<UInt64> lost;
	UInt64 current=_stage;
	UInt32 count=0;
	map<UInt64,RTMFPFragment*>::const_iterator it=_fragments.begin();
	while(it!=_fragments.end()) {
		current = it->first-current-2;
		size += Util::Get7BitValueSize(current);
		lost.push_back(current);
		current = it->first;
		while(++it!=_fragments.end() && it->first==(++current))
			++count;
		size += Util::Get7BitValueSize(count);
		lost.push_back(count);
		--current;
		count=0;
	}

	UInt32 bufferSize = _pPacket ? ((_pPacket->fragments>0x3F00) ? 0 : (0x3F00-_pPacket->fragments)) : 0x7F;
	if(_pStream.isNull())
		bufferSize=0;

	Exception ex;
	MemoryWriter& ack = _band.writeMessage(ex, 0x51,Util::Get7BitValueSize(id)+Util::Get7BitValueSize(bufferSize)+Util::Get7BitValueSize(_stage)+size);
	if (ex)
		ERROR("Error while trying to write message : ", ex.error());

	UInt32 pos = ack.position();
	ack.write7BitLongValue(id);
	ack.write7BitValue(bufferSize);
	ack.write7BitLongValue(_stage);

	list<UInt64>::const_iterator it2;
	for(it2=lost.begin();it2!=lost.end();++it2)
		ack.write7BitLongValue(*it2);

	if(!_pStream.isNull())
		_pStream->flush();

	Exception exf;
	_pWriter->flush(exf);
	ERROR("Error while trying to flush message : ", ex.error());
}

void RTMFPFlow::fragmentHandler(UInt64 _stage,UInt64 deltaNAck,MemoryReader& fragment,UInt8 flags) {
	if(_completed)
		return;

	if(_pStream.isNull()) { // if this==FlowNull
		fail("RTMFPMessage received for a RTMFPFlow unknown");
		(UInt64&)this->_stage = _stage;
		return;
	}

//	TRACE("RTMFPFlow ",id," _stage ",_stage);

	UInt64 nextStage = this->_stage+1;

	if(_stage < nextStage) {
		DEBUG("Stage ",_stage," on flow ",id," has already been received");
		return;
	}

	if(deltaNAck>_stage) {
		WARN("DeltaNAck ",deltaNAck," superior to _stage ",_stage," on flow ",id);
		deltaNAck=_stage;
	}
	
	if(this->_stage < (_stage-deltaNAck)) {
		map<UInt64,RTMFPFragment*>::iterator it=_fragments.begin();
		while(it!=_fragments.end()) {
			if( it->first > _stage) 
				break;
			// leave all stages <= _stage
			MemoryReader reader(it->second->begin(),it->second->size());
			fragmentSortedHandler(it->first,reader,it->second->flags);
			if(it->second->flags&MESSAGE_END) {
				complete();
				return; // to prevent a crash bug!! (double fragments deletion)
			}
			delete it->second;
			_fragments.erase(it++);
		}

		nextStage = _stage;
	}
	
	if(_stage>nextStage) {
		// not following _stage, bufferizes the _stage
		map<UInt64,RTMFPFragment*>::iterator it = _fragments.lower_bound(_stage);
		if(it==_fragments.end() || it->first!=_stage) {
			if(it!=_fragments.begin())
				--it;
			_fragments.insert(it,pair<UInt64,RTMFPFragment*>(_stage,new RTMFPFragment(fragment,flags)));
			if(_fragments.size()>100)
				DEBUG("_fragments.size()=",_fragments.size());
		} else
			DEBUG("Stage ",_stage," on flow ",id," has already been received");
	} else {
		fragmentSortedHandler(nextStage++,fragment,flags);
		if(flags&MESSAGE_END)
			complete();
		map<UInt64,RTMFPFragment*>::iterator it=_fragments.begin();
		while(it!=_fragments.end()) {
			if( it->first > nextStage)
				break;
			MemoryReader reader(it->second->begin(),it->second->size());
			fragmentSortedHandler(nextStage++,reader,it->second->flags);
			if(it->second->flags&MESSAGE_END) {
				complete();
				return; // to prevent a crash bug!! (double fragments deletion)
			}
			delete it->second;
			_fragments.erase(it++);
		}

	}
}

void RTMFPFlow::fragmentSortedHandler(UInt64 _stage,MemoryReader& fragment,UInt8 flags) {
	if(_stage<=this->_stage) {
		ERROR("Stage ",_stage," not sorted on flow ",id);
		return;
	}
	if(_stage>(this->_stage+1)) {
		// not following _stage!
		UInt32 lostCount = (UInt32)(_stage-this->_stage-1);
		(UInt64&)this->_stage = _stage;
		if(_pPacket) {
			delete _pPacket;
			_pPacket = NULL;
		}
		if(flags&MESSAGE_WITH_BEFOREPART) {
			_numberLostFragments += (lostCount+1);
			return;
		}
		_numberLostFragments += lostCount;
	} else
		(UInt64&)this->_stage = _stage;

	// If MESSAGE_ABANDONMENT, content is not the right normal content!
	if(flags&MESSAGE_ABANDONMENT) {
		if(_pPacket) {
			delete _pPacket;
			_pPacket = NULL;
		}
		return;
	}

	MemoryReader* pMessage(&fragment);
	if(flags&MESSAGE_WITH_BEFOREPART){
		if(!_pPacket) {
			WARN("A received message tells to have a 'beforepart' and nevertheless partbuffer is empty, certainly some packets were lost");
			++_numberLostFragments;
			delete _pPacket;
			_pPacket = NULL;
			return;
		}
		
		_pPacket->add(fragment);

		if(flags&MESSAGE_WITH_AFTERPART)
			return;

		pMessage = _pPacket->release();
	} else if(flags&MESSAGE_WITH_AFTERPART) {
		if(_pPacket) {
			ERROR("A received message tells to have not 'beforepart' and nevertheless partbuffer exists");
			_numberLostFragments += _pPacket->fragments;
			delete _pPacket;
		}
		_pPacket = new RTMFPPacket(fragment);
		return;
	}

	_pStream->process(unpack(*pMessage),*pMessage,*_pWriter,_numberLostFragments);
	_numberLostFragments=0;

	if(_pPacket) {
		delete _pPacket;
		_pPacket=NULL;
	}
	
}


} // namespace Mona
