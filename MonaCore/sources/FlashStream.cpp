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
#include "Mona/Logs.h"


using namespace std;

namespace Mona {

FlashStream::FlashStream(UInt16 id, Invoker& invoker, Peer& peer) : id(id), invoker(invoker), peer(peer), _pPublication(NULL), _pListener(NULL), _bufferTime(0) {
	DEBUG("FlashStream ",id," created")
}

FlashStream::~FlashStream() {
	disengage();
	DEBUG("FlashStream ",id," deleted")
}

void FlashStream::disengage(FlashWriter* pWriter) {
	// Stop the current  job
	if(_pPublication) {
		const string& name(_pPublication->name());
		if(pWriter)
			pWriter->writeAMFStatus("NetStream.Unpublish.Success",name + " is now unpublished");
		 // do after writeAMFStatus because can delete the publication, so corrupt name reference
		invoker.unpublish(peer,name);
		_pPublication = NULL;
	}
	if(_pListener) {
		const string& name(_pListener->publication.name());
		if (pWriter) {
			pWriter->writeAMFStatus("NetStream.Play.Stop", "Stopped playing " + name);
			OnStop::raise(id, *pWriter); // stream end
		}
		 // do after writeAMFStatus because can delete the publication, so corrupt publication name reference
		invoker.unsubscribe(peer,name);
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
		_pListener = invoker.subscribe(ex, peer, publication, writer); // ex already log displayed
		if (!_pListener) {
			writer.writeAMFStatus("NetStream.Play.Failed", ex.error());
			return;
		}
		
		OnStart::raise(id, writer); // stream begin
		writer.writeAMFStatus("NetStream.Play.Reset", "Playing and resetting " + publication); // for entiere playlist
		writer.writeAMFStatus("NetStream.Play.Start", "Started playing "+publication); // for item
		AMFWriter& amf(writer.writeInfos("|RtmpSampleAccess"));
		amf.writeBoolean(true); // audioSampleAccess
		amf.writeBoolean(true); // videoSampleAccess

		if (_bufferTime > 0)
			_pListener->setNumber("bufferTime", _bufferTime);
	
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

	} else if (_pListener && name == "pause") {
		bool paused(true);
		message.readBoolean(paused);
		// TODO support pause for VOD
		
		if (paused) {
			// useless, client knows it when it calls NetStream::pause method
			// writer.writeAMFStatus("NetStream.Pause.Notify", _pListener->publication.name() + " paused");
		} else {
			double position;
			if (message.readNumber(position))
				_pListener->seek((UInt32)position);
			OnStart::raise(id, writer); // stream begin
			// useless, client knows it when it calls NetStream::resume method
			//	writer.writeAMFStatus("NetStream.Unpause.Notify", _pListener->publication.name() + " resumed");
		}

	} else if (_pListener && name == "seek") {
		double position;
		if (message.readNumber(position)) {
			_pListener->seek((UInt32)position);
			 // TODO support seek for VOD
			OnStart::raise(id, writer); // stream begin
			// useless, client knows it when it calls NetStream::seek method, and wait "NetStream.Seek.Complete" rather (raised by client side)
			// writer.writeAMFStatus("NetStream.Seek.Notify", _pListener->publication.name() + " seek operation");
		} else
			writer.writeAMFStatus("NetStream.Seek.InvalidTime", _pListener->publication.name() + " seek operation must pass in argument a milliseconds position time");
	} else if (_pPublication && name == "@setDataFrame") {
		// metadata
		_pPublication->writeProperties(message);
	} else if (_pPublication && name == "@clearDataFrame") {
		_pPublication->clearProperties();
	} else
		ERROR("Message '",name,"' unknown on stream ",id);
}


void FlashStream::rawHandler(UInt8 type, PacketReader& packet, FlashWriter& writer) {
	if(packet.read16()==0x22) { // TODO Here we receive RTMFP flow sync signal, useless to support it!
		//TRACE("Sync ",id," : ",data.read32(),"/",data.read32());
		return;
	}
	ERROR("Raw message ",Format<UInt8>("%.2x",type),"/",packet.read16()," unknown on stream ",id);
}

UInt32 FlashStream::bufferTime(UInt32 ms) {
	_bufferTime = ms;
	INFO("setBufferTime ", ms, "ms on stream ",id)
	if (_pListener)
		_pListener->setNumber("bufferTime", ms);
	return _bufferTime;
}

void FlashStream::dataHandler(DataReader& data, double lostRate) {
	if(!_pPublication) {
		ERROR("a data packet has been received on a no publishing stream ",id,", certainly a publication currently closing");
		return;
	}

	// necessary AMF0 here!
	if (*data.packet.current() == AMF_STRING && *(data.packet.current() + 3) == '@') {
		
		if (*(data.packet.current() + 1) == 0) {
			if (*(data.packet.current() + 2) == 13 && memcmp(data.packet.current() + 3, EXPAND("@setDataFrame"))==0) {
				// @setDataFrame
				data.next();
				_pPublication->writeProperties(data);
				return;
			} else if (*(data.packet.current() + 2) == 15 && memcmp(data.packet.current() + 3, EXPAND("@clearDataFrame"))==0) {
				// @clearDataFrame
				_pPublication->clearProperties();
				return;
			}
		}
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


} // namespace Mona
