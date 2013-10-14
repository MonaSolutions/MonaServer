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

#include "Mona/FlashStream.h"
#include "Mona/Invoker.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"
#include "Poco/Format.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {


FlashStream::FlashStream(Invoker& invoker,Peer& peer) : id(0),invoker(invoker),peer(peer),_pPublication(NULL),_pListener(NULL),_bufferTime(0) {
	DEBUG("FlashStream %u created",id)
}

FlashStream::~FlashStream() {
	disengage();
	DEBUG("FlashStream %u deleted",id)
}

void FlashStream::disengage(FlashWriter* pWriter) {
	// Stop the current  job
	if(_pPublication) {
		string name = _pPublication->name();
		invoker.unpublish(peer,name);
		if(pWriter)
			pWriter->writeAMFStatus("NetStream.Unpublish.Success",name + " is now unpublished");
		_pPublication = NULL;
	}
	if(_pListener) {
		string name = _pListener->publication.name();
		invoker.unsubscribe(peer,name);
		if(pWriter)
			pWriter->writeAMFStatus("NetStream.Play.Stop","Stopped playing " + name);
		_pListener = NULL;
	}
}

void FlashStream::close(FlashWriter& writer,const string& error,int code) {
	// TODO
}

void FlashStream::process(AMF::ContentType type,MemoryReader& data,FlashWriter& writer,UInt32 numberLostFragments) {
	if(type==AMF::EMPTY)
		return;
	
	writer.callbackHandle = 0;
	try {
		switch(type) {
			case AMF::INVOCATION: {
				string name;
				AMFReader reader(data);
				reader.readString(name);
				writer.callbackHandle = reader.readNumber();
				if(reader.followingType()==AMFReader::NIL)
					reader.readNull();
				messageHandler(name,reader,writer);
				break;
			}
			case AMF::DATA: {
				AMFReader reader(data);
				dataHandler(reader,numberLostFragments);
				break;
			}
			case AMF::AUDIO:
				audioHandler(data,numberLostFragments);
				break;
			case AMF::VIDEO:
				videoHandler(data,numberLostFragments);
				break;
			default:
				rawHandler(type,data,writer);
		}
	} catch(exception& ex) {
		close(writer,ex.what());
	} catch(...) {
		close(writer,"Unknown error");
	}
	writer.callbackHandle = 0;
}


void FlashStream::messageHandler(const string& name,AMFReader& message,FlashWriter& writer) {
	if(name=="play") {
		disengage(&writer);

		string publication;
		message.readString(publication);
		// TODO implements completly NetStream.play method, with possible NetStream.play.failed too!
		double start = -2000;
		if(message.available())
			start = message.readNumber();

		try {
			_pListener = &invoker.subscribe(peer,publication,writer,start);
			if(_bufferTime>0) {
				// To do working the buffertime on receiver side
				BinaryWriter& raw = writer.writeRaw();
				raw.write16(0);
				raw.write32(id);
				_pListener->setBufferTime(_bufferTime);
			}
			writer.writeAMFStatus("NetStream.Play.Reset","Playing and resetting " + publication);
			writer.writeAMFStatus("NetStream.Play.Start","Started playing " + publication);
		} catch(Exception& ex) {
			writer.writeAMFStatus("NetStream.Play.Failed",ex.message());
		}
		
	} else if(name == "closeStream") {
		disengage(&writer);
	} else if(name=="publish") {

		disengage(&writer);

		string type,publication;
		message.readString(publication);
		if(message.available())
			message.readString(type); // TODO recording publication feature!

		try {
			_pPublication = &invoker.publish(peer,publication);
			if(_bufferTime>0)
				_pPublication->setBufferTime(_bufferTime);
			writer.writeAMFStatus("NetStream.Publish.Start",publication +" is now published");
		} catch(Exception& ex) {
			writer.writeAMFStatus("NetStream.Publish.BadName",ex.message());
		}
	} else if(_pListener && name=="receiveAudio") {
		_pListener->receiveAudio = message.readBoolean();
	} else if(_pListener && name=="receiveVideo") {
		_pListener->receiveVideo = message.readBoolean();
	} else
		ERROR("RTMFPMessage '%s' unknown on stream %u",name.c_str(),id);
	
}


void FlashStream::rawHandler(UInt8 type,MemoryReader& data,FlashWriter& writer) {
	if(data.read16()==0x22) { // TODO Here we receive publication bounds (id + tracks), useless? maybe to record a file and sync tracks?
		//TRACE("Bound %u : %u %u",id,data.read32(),data.read32());
		return;
	}
	ERROR("Raw message %u unknown on stream %u",type,id);
}

void FlashStream::setBufferTime(UInt32 ms) {
	_bufferTime = ms;
	if(_pPublication)
		_pPublication->setBufferTime(ms);
	if(_pListener)
		_pListener->setBufferTime(ms);
}

void FlashStream::dataHandler(DataReader& data,UInt32 numberLostFragments) {
	if(!_pPublication) {
		ERROR("a data packet has been received on a no publishing stream %u",id);
		return;
	}
	if(_pPublication->publisher() == &peer)
		_pPublication->pushData(data,numberLostFragments);
	else
		WARN("a data packet has been received on a stream %u which is not on owner of this publication, certainly a publication currently closing",id);
}


void FlashStream::audioHandler(MemoryReader& packet,UInt32 numberLostFragments) {
	if(!_pPublication) {
		WARN("an audio packet has been received on a no publishing stream %u, certainly a publication currently closing",id);
		return;
	}
	if(_pPublication->publisher() == &peer)
		_pPublication->pushAudio(packet,packet.read32(),numberLostFragments);
	else
		WARN("an audio packet has been received on a stream %u which is not on owner of this publication, certainly a publication currently closing",id);
}

void FlashStream::videoHandler(MemoryReader& packet,UInt32 numberLostFragments) {
	if(!_pPublication) {
		WARN("a video packet has been received on a no publishing stream %u, certainly a publication currently closing",id);
		return;
	}
	if(_pPublication->publisher() == &peer)
		_pPublication->pushVideo(packet,packet.read32(),numberLostFragments);
	else
		WARN("a video packet has been received on a stream %u which is not on owner of this publication, certainly a publication currently closing",id);
}

void FlashStream::flush() {
	if(_pPublication && _pPublication->publisher() == &peer)
		_pPublication->flush();
}


} // namespace Mona
