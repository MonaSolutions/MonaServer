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

namespace Mona {


FlashStream::FlashStream(UInt32 id, Invoker& invoker, Peer& peer) : id(id), invoker(invoker), peer(peer), _pPublication(NULL), _pListener(NULL), _bufferTime(0) {
	DEBUG("FlashStream ",id," created")
}

FlashStream::~FlashStream() {
	disengage();
	DEBUG("FlashStream ",id," deleted")
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
	Exception ex;
	switch(type) {
		case AMF::INVOCATION: {
			string name;
			AMFReader reader(data);
			reader.readString(name);
			writer.callbackHandle = reader.readNumber();
			if(reader.followingType()==AMFReader::NIL)
				reader.readNull();
			messageHandler(ex,name,reader,writer);
			break;
		}
		case AMF::DATA: {
			AMFReader reader(data);
			dataHandler(ex, reader, numberLostFragments);
			break;
		}
		case AMF::AUDIO:
			audioHandler(ex, data, numberLostFragments);
			break;
		case AMF::VIDEO:
			videoHandler(ex, data, numberLostFragments);
			break;
		default:
			rawHandler(ex, type, data, writer);
	}
	if (ex)
		close(writer,ex.error());
	writer.callbackHandle = 0;
}


void FlashStream::messageHandler(Exception& ex,const string& name,AMFReader& message,FlashWriter& writer) {
	if(name=="play") {
		disengage(&writer);

		string publication;
		message.readString(publication);
		// TODO implements completly NetStream.play method, with possible NetStream.play.failed too!
		double start = -2000;
		if(message.available())
			start = message.readNumber();

		Exception ex;
		_pListener = invoker.subscribe(ex,peer,publication,writer,start);
		if (ex) {
			writer.writeAMFStatus("NetStream.Play.Failed",ex.error());
			return;
		}

		if(_bufferTime>0) {
			// To do working the buffertime on receiver side
			BinaryWriter& raw = writer.writeRaw();
			raw.write16(0);
			raw.write32(id);
			_pListener->setBufferTime(_bufferTime);
		}
		writer.writeAMFStatus("NetStream.Play.Reset","Playing and resetting " + publication);
		writer.writeAMFStatus("NetStream.Play.Start","Started playing " + publication);
		
	} else if(name == "closeStream") {
		disengage(&writer);
	} else if(name=="publish") {

		disengage(&writer);

		string type,publication;
		message.readString(publication);
		if(message.available())
			message.readString(type); // TODO recording publication feature!

		Exception ex;
		_pPublication = invoker.publish(ex, peer,publication);
		if (ex)
			writer.writeAMFStatus("NetStream.Publish.BadName",ex.error());
		else {
			if(_bufferTime>0)
				_pPublication->setBufferTime(_bufferTime);
			writer.writeAMFStatus("NetStream.Publish.Start",publication +" is now published");
		}
	} else if(_pListener && name=="receiveAudio") {
		_pListener->receiveAudio = message.readBoolean();
	} else if(_pListener && name=="receiveVideo") {
		_pListener->receiveVideo = message.readBoolean();
	} else
		ERROR("RTMFPMessage '",name,"' unknown on stream ",id);
	
}


void FlashStream::rawHandler(Exception& ex, UInt8 type, MemoryReader& data, FlashWriter& writer) {
	if(data.read16()==0x22) { // TODO Here we receive publication bounds (id + tracks), useless? maybe to record a file and sync tracks?
		//TRACE("Bound ",id," : ",data.read32()," ",data.read32());
		return;
	}
	ERROR("Raw message ",type," unknown on stream ",id);
}

void FlashStream::setBufferTime(UInt32 ms) {
	_bufferTime = ms;
	if(_pPublication)
		_pPublication->setBufferTime(ms);
	if(_pListener)
		_pListener->setBufferTime(ms);
}

void FlashStream::dataHandler(Exception& ex, DataReader& data, UInt32 numberLostFragments) {
	if(!_pPublication) {
		ERROR("a data packet has been received on a no publishing stream ",id);
		return;
	}
	if(_pPublication->publisher() == &peer)
		_pPublication->pushData(data,numberLostFragments);
	else
		WARN("a data packet has been received on a stream ",id," which is not on owner of this publication, certainly a publication currently closing");
}


void FlashStream::audioHandler(Exception& ex, MemoryReader& packet, UInt32 numberLostFragments) {
	if(!_pPublication) {
		WARN("an audio packet has been received on a no publishing stream ",id,", certainly a publication currently closing");
		return;
	}
	if(_pPublication->publisher() == &peer)
		_pPublication->pushAudio(packet,packet.read32(),numberLostFragments);
	else
		WARN("an audio packet has been received on a stream ",id," which is not on owner of this publication, certainly a publication currently closing");
}

void FlashStream::videoHandler(Exception& ex, MemoryReader& packet, UInt32 numberLostFragments) {
	if(!_pPublication) {
		WARN("a video packet has been received on a no publishing stream ",id,", certainly a publication currently closing");
		return;
	}
	if(_pPublication->publisher() == &peer)
		_pPublication->pushVideo(packet,packet.read32(),numberLostFragments);
	else
		WARN("a video packet has been received on a stream ",id," which is not on owner of this publication, certainly a publication currently closing");
}

void FlashStream::flush() {
	if(_pPublication && _pPublication->publisher() == &peer)
		_pPublication->flush();
}


} // namespace Mona
