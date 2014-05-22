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

#include "Mona/FlashStream.h"
#include "Mona/Invoker.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"


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

bool FlashStream::process(AMF::ContentType type,UInt32 time,PacketReader& packet,FlashWriter& writer,UInt32 numberLostFragments) {
	if(type==AMF::EMPTY)
		return writer.state()!=Writer::CLOSED;

	// if exception, it closes the connection, and print an ERROR message
	switch(type) {
		case AMF::INVOCATION_AMF3:
		case AMF::INVOCATION: {
			string name;
			AMFReader reader(packet);
			reader.readString(name);
			writer.callbackHandle = reader.readNumber();
			if(reader.followingType()==AMFReader::NIL)
				reader.readNull();
			messageHandler(name,reader,writer);
			break;
		}
		case AMF::DATA: {
			AMFReader reader(packet);
			dataHandler(reader, numberLostFragments);
			break;
		}
		case AMF::AUDIO:
			audioHandler(time,packet, numberLostFragments);
			break;
		case AMF::VIDEO:
			videoHandler(time,packet, numberLostFragments);
			break;
		case AMF::ACK:
			// nothing to do, a ack message says about how many bytes have been gotten by the peer
			// RTMFP has a complexe ack mechanism and RTMP is TCP based, ack mechanism is in system layer => so useless
			break;
		default:
			rawHandler(type, packet, writer);
	}
	writer.callbackHandle = 0;
	return writer.state()!=Writer::CLOSED;
}


void FlashStream::messageHandler(const string& name,AMFReader& message,FlashWriter& writer) {
	if(name=="play") {
		disengage(&writer);

		string publication;
		message.readString(publication);
		// TODO implements completly NetStream.play method, with possible NetStream.play.failed too!
		Exception ex;
		_pListener = invoker.subscribe(ex,peer,publication,writer);
		if (ex) {
			writer.writeAMFStatus("NetStream.Play.Failed",ex.error());
			return;
		}
		if(message.available())
			_pListener->setNumber("unbuffered",message.readNumber()==-3000);

		if(_bufferTime>0) {
			// To do working the buffertime on receiver side
			BinaryWriter& raw = writer.writeRaw();
			raw.write16(0);
			raw.write32(id);
			_pListener->setBufferTime(_bufferTime);
		}
		writer.writeAMFStatus("NetStream.Play.Reset","Playing and resetting " + publication); // for entiere playlist
		writer.writeAMFStatus("NetStream.Play.Start","Started playing " + publication); // for item

	} else if(name == "closeStream") {
		disengage(&writer);
	} else if(name=="publish") {

		disengage(&writer);

		string type,publication;
		message.readString(publication);
		size_t query = publication.find('?');
		if (query != string::npos)
			publication = publication.substr(0, query); // TODO use query in Util::UnpackQuery for publication options?
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


void FlashStream::rawHandler(UInt8 type, PacketReader& packet, FlashWriter& writer) {
	if(packet.read16()==0x22) { // TODO Here we receive publication bounds (id + tracks), useless? maybe to record a file and sync tracks?
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

void FlashStream::dataHandler(DataReader& data, UInt32 numberLostFragments) {
	if(!_pPublication) {
		ERROR("a data packet has been received on a no publishing stream ",id);
		return;
	}
	if(_pPublication->publisher() == &peer)
		_pPublication->pushData(data,numberLostFragments);
	else
		WARN("a data packet has been received on a stream ",id," which is not on owner of this publication, certainly a publication currently closing");
}


void FlashStream::audioHandler(UInt32 time,PacketReader& packet, UInt32 numberLostFragments) {
	if(!_pPublication) {
		WARN("an audio packet has been received on a no publishing stream ",id,", certainly a publication currently closing");
		return;
	}
	if(_pPublication->publisher() == &peer)
		_pPublication->pushAudio(time,packet,numberLostFragments);
	else
		WARN("an audio packet has been received on a stream ",id," which is not on owner of this publication, certainly a publication currently closing");
}

void FlashStream::videoHandler(UInt32 time,PacketReader& packet, UInt32 numberLostFragments) {
	if(!_pPublication) {
		WARN("a video packet has been received on a no publishing stream ",id,", certainly a publication currently closing");
		return;
	}
	if(_pPublication->publisher() == &peer)
		_pPublication->pushVideo(time,packet,numberLostFragments);
	else
		WARN("a video packet has been received on a stream ",id," which is not on owner of this publication, certainly a publication currently closing");
}

void FlashStream::flush() {
	if(_pPublication && _pPublication->publisher() == &peer)
		_pPublication->flush();
}


} // namespace Mona
