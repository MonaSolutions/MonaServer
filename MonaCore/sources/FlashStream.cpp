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
#include "Mona/ParameterWriter.h"
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

bool FlashStream::process(AMF::ContentType type,UInt32 time,PacketReader& packet,FlashWriter& writer,double lostRate) {
	if(type==AMF::EMPTY)
		return writer.state()!=Writer::CLOSED;

	// if exception, it closes the connection, and print an ERROR message
	switch(type) {
		case AMF::INFORMATIONS: {
			string name;
			AMFReader reader(packet);
			reader.readString(name);
			messageHandler(name,reader,writer);
			break;
		}
		case AMF::INVOCATION_AMF3:
		case AMF::INVOCATION: {
			string name;
			AMFReader reader(packet);
			reader.readString(name);
			double number(0);
			reader.readNumber(number);
			writer.setCallbackHandle(number);
			reader.readNull();
			messageHandler(name,reader,writer);
			break;
		}
		case AMF::DATA: {
			AMFReader reader(packet);
			dataHandler(reader, lostRate);
			break;
		}
		case AMF::AUDIO:
			audioHandler(time,packet, lostRate);
			break;
		case AMF::VIDEO:
			videoHandler(time,packet, lostRate);
			break;
		
			break;
		case AMF::ACK:
			// nothing to do, a ack message says about how many bytes have been gotten by the peer
			// RTMFP has a complexe ack mechanism and RTMP is TCP based, ack mechanism is in system layer => so useless
			break;
		default:
			rawHandler(type, packet, writer);
	}
	writer.setCallbackHandle(0);
	return writer.state()!=Writer::CLOSED;
}


void FlashStream::messageHandler(const string& name, AMFReader& message, FlashWriter& writer) {
	if (name == "play") {
		disengage(&writer);

		string publication;
		message.readString(publication);
		// TODO implements completly NetStream.play method, with possible NetStream.play.failed too!
		Exception ex;
		_pListener = invoker.subscribe(ex, peer, publication, writer);
		if (ex) {
			writer.writeAMFStatus("NetStream.Play.Failed", ex.error());
			return;
		}
		double number;
		if (message.readNumber(number))
			_pListener->setNumber("unbuffered", number == -3000);

		if (_bufferTime > 0) {
			// To do working the buffertime on receiver side
			BinaryWriter& raw = writer.writeRaw();
			raw.write16(0);
			raw.write32(id);
			_pListener->setNumber("bufferTime", _bufferTime);
		}
		writer.writeAMFStatus("NetStream.Play.Reset", "Playing and resetting " + publication); // for entiere playlist
		writer.writeAMFStatus("NetStream.Play.Start", "Started playing " + publication); // for item

	} else if (name == "closeStream") {
		disengage(&writer);
	} else if (name == "publish") {

		disengage(&writer);

		string type, publication;
		message.readString(publication);
		size_t query = publication.find('?');
		if (query != string::npos)
			publication = publication.substr(0, query); // TODO use query in Util::UnpackQuery for publication options?
		if (message.available())
			message.readString(type); // TODO support "append" and "appendWithGap"

		Exception ex;
		_pPublication = invoker.publish(ex, peer, publication, type == "record" ? Publication::RECORD : Publication::LIVE);
		if (ex) {
			writer.writeAMFStatus("NetStream.Publish.BadName", ex.error());
			_pPublication = NULL;
		} else
			writer.writeAMFStatus("NetStream.Publish.Start", publication + " is now published");
	} else if (_pListener && name == "receiveAudio") {
		message.readBoolean(_pListener->receiveAudio);
	} else if (_pListener && name == "receiveVideo") {
		message.readBoolean(_pListener->receiveVideo);
	} else if (_pPublication && name == "@setDataFrame") {
		// metadata
		string handler;
		message.readString(handler);
		_pPublication->writeProperties(handler.c_str(), message);
	} else if (_pPublication && name == "@clearDataFrame") {
		_pPublication->clearProperties();
	} else
		ERROR("Message '",name,"' unknown on stream ",id);
}


void FlashStream::rawHandler(UInt8 type, PacketReader& packet, FlashWriter& writer) {
	if(packet.read16()==0x22) { // TODO Here we receive publication bounds (id + tracks), useless? maybe to record a file and sync tracks?
		//TRACE("Bound ",id," : ",data.read32()," ",data.read32());
		return;
	}
	ERROR("Raw message ",type," unknown on stream ",id);
}

UInt32 FlashStream::bufferTime(UInt32 ms) {
	_bufferTime = ms;
	INFO("setBufferTime ", ms, "ms on stream ",id)
	if(_pListener)
		_pListener->setNumber("bufferTime",ms);
	return _bufferTime;
}

void FlashStream::dataHandler(DataReader& data, double lostRate) {
	if(!_pPublication) {
		ERROR("a data packet has been received on a no publishing stream ",id,", certainly a publication currently closing");
		return;
	}
	_pPublication->pushData(data,peer.ping(),lostRate);
}


void FlashStream::audioHandler(UInt32 time,PacketReader& packet, double lostRate) {
	if(!_pPublication) {
		WARN("an audio packet has been received on a no publishing stream ",id,", certainly a publication currently closing");
		return;
	}
	_pPublication->pushAudio(time,packet,peer.ping(),lostRate);
}

void FlashStream::videoHandler(UInt32 time,PacketReader& packet, double lostRate) {
	if(!_pPublication) {
		WARN("a video packet has been received on a no publishing stream ",id,", certainly a publication currently closing");
		return;
	}
	_pPublication->pushVideo(time,packet,peer.ping(),lostRate);
}

void FlashStream::flush() {
	if(_pPublication)
		_pPublication->flush();
}


} // namespace Mona
