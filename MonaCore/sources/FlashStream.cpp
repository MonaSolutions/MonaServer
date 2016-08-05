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
#include "Room.h"
#include "SharedObject.h"
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
		peer.room->unpublish(peer,name);
		//invoker.unpublish(peer,name);
		_pPublication = NULL;
	}
	if(_pListener) {
		const string& name(_pListener->publication.name());
		if (pWriter) {
			pWriter->writeAMFStatus("NetStream.Play.Stop", "Stopped playing " + name);
			OnStop::raise(id, *pWriter); // stream end
		}
		 // do after writeAMFStatus because can delete the publication, so corrupt publication name reference
		//invoker.unsubscribe(peer,name);
		peer.room->unsubscribe(peer, name);
		_pListener = NULL;
	}
}


bool FlashStream::process(AMF::ContentType type,UInt32 time,PacketReader& packet,FlashWriter& writer,double lostRate) {

	// if exception, it closes the connection, and print an ERROR message
	switch(type) {

		case AMF::AUDIO:
			audioHandler(time,packet, lostRate);
			break;
		case AMF::VIDEO:
			videoHandler(time,packet, lostRate);
			break;

		case AMF::DATA_AMF3:
			packet.next();
		case AMF::DATA: {
			AMFReader reader(packet);
			dataHandler(reader, lostRate);
			break;
		}

		case AMF::EMPTY:
			break;

		case AMF::INVOCATION_AMF3:
			packet.next();
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

		case AMF::RAW:
			rawHandler(packet.read16(), packet, writer);
			break;
		case AMF::FLEXSHAREDOBJECT:
			packet.read8();
		case AMF::SHAREDOBJECT:
		{
			AMFReader reader(packet);
			string name = reader.readString();
			uint32_t currentVersion = packet.read32();
			uint32_t persistence = packet.read32();
			packet.read32();
			SharedObject* SO = peer.room->getSO(name, persistence == 2);
			while (packet.available() > 0) {
				AMF::SharedObjectType type = static_cast<AMF::SharedObjectType>(packet.read8());
				uint32_t rawLength = packet.read32();
				uint32_t endPosition = packet.position() + rawLength;
				switch (type) {
				case AMF::CS_CONNECT:
					SO->onClientConnect(peer.id);
					_sos.insert(SO);
					break;
				case AMF::CS_DISCONNECT:
					SO->onClientDisconnect(peer.id);
					_sos.erase(SO);
					break;
				case AMF::CSC_DELETE_DATA:
					SO->unSet(reader.readString());
					break;
				case AMF::CS_SET_ATTRIBUTE:
					while (packet.position() < endPosition) {
						name = reader.readString();
						AMFObjectWriter amfo_writer(invoker.poolBuffers);
						reader.read(amfo_writer,1);
						SO->set(name, move(amfo_writer.amfObject), peer.id);
					}
					break;
				case AMF::BW_SEND_MESSAGE:
					//while (rtmpBody.offset < endPosition) {
					//	AMFObject(rtmpBody);
					//	//todo send message
					//}
					break;
				default:
					break;
				}
			}
			if (SO)SO->track();
			break;
		}
		default:
			ERROR("Unpacking type '",Format<UInt8>("%02x",(UInt8)type),"' unknown");
	}

	writer.setCallbackHandle(0);
	return writer.state()!=Writer::CLOSED;
}


UInt32 FlashStream::bufferTime(UInt32 ms) {
	_bufferTime = ms;
	INFO("setBufferTime ", ms, "ms on stream ",id)
	if (_pListener)
		_pListener->setNumber("bufferTime", ms);
	return _bufferTime;
}

void FlashStream::messageHandler(const string& name, AMFReader& message, FlashWriter& writer) {
	if (name == "play") {
		disengage(&writer);

		string publication;
		double startTime = -2000;
		double length = -1;
		message.readString(publication);
		if (message.available())
		{
			message.readNumber(startTime);
		}
		if (message.available())
		{
			message.readNumber(length);
		}

		// TODO implements completly NetStream.play method, with possible NetStream.play.failed too!
		Exception ex;
		//_pListener = invoker.subscribe(ex, peer, publication, writer); // ex already log displayed
		_pListener = peer.room->subscribe(ex, peer, publication, writer);
		double duration = 0;
		if (!_pListener) {
			writer.writeAMFStatus("NetStream.Play.Failed", ex.error());
			return;
		}
		else
		{
			if (startTime != -1000 && !_pListener->publication.running())
			{
				string filePath("./www" + peer.path + "/media/" + publication + ".flv");
				if (FileSystem::Exists(filePath))
				{
					peer.room->unsubscribe(peer, publication);
					publication = publication + (char*)peer.id;
					_pListener = peer.room->subscribe(ex, peer, publication, writer);

					auto flvReader = new InFileRTMPStream(filePath, &invoker, (Publication*)&_pListener->publication, &writer);
					duration = flvReader->duration;
					if (startTime > 0)
					{
						flvReader->seek(startTime);
					}
					flvReader->pThread = invoker.poolThreads.enqueue(ex, shared_ptr<InFileRTMPStream>(flvReader), flvReader->pThread);
				}
				else
				{
					writer.writeAMFStatus("NetStream.Play.StreamNotFound", ex.error());
					return;
				}
			}
		}

		OnStart::raise(id, writer); // stream begin
		writer.writeAMFStatus("NetStream.Play.Reset", "Playing and resetting " + publication); // for entiere playlist
		writer.writeAMFStatus("NetStream.Play.Start", "Started playing " + publication); // for item
		AMFWriter& amf(writer.writeAMFData("|RtmpSampleAccess"));
		amf.writeBoolean(true); // audioSampleAccess
		amf.writeBoolean(true); // videoSampleAccess
		if (duration){
			auto& onMetaData = writer.writeAMFData("onMetaData");
			onMetaData.amf0 = true;
			onMetaData.beginObject();
			onMetaData.writePropertyName("duration");
			onMetaData.writeNumber(duration/1000.0);
			onMetaData.endObject();
		}
		if (_bufferTime > 0)
			_pListener->setNumber("bufferTime", _bufferTime);
		return;
	}
	
	if (name == "closeStream") {
		disengage(&writer);
		return;
	}
	
	if (name == "publish") {

		disengage(&writer);

		string type, publication;
		message.readString(publication);
		size_t query = publication.find('?');
		if (query != string::npos)
			publication = publication.substr(0, query); // TODO use query in Util::UnpackQuery for publication options?
		if (message.available())
			message.readString(type); // TODO support "append" and "appendWithGap"

		Exception ex;
		//_pPublication = invoker.publish(ex, peer, publication, type == "record" ? Publication::RECORD : type == "append"? Publication::APPEND : Publication::LIVE);
		_pPublication = peer.room->publish(ex, peer, publication, type == "record" ? Publication::RECORD : type == "append" ? Publication::APPEND : Publication::LIVE);
		if (ex) {
			writer.writeAMFStatus("NetStream.Publish.BadName", ex.error());
			_pPublication = NULL;
		} else
			writer.writeAMFStatus("NetStream.Publish.Start", publication + " is now published");
		return;
	}
	
	if (_pListener) {

		if(name == "receiveAudio") {
			message.readBoolean(_pListener->receiveAudio);
			return;
		}
		
		if (name == "receiveVideo") {
			message.readBoolean(_pListener->receiveVideo);
			return;
		}
		
		if (name == "pause") {
			bool paused(true);
			message.readBoolean(paused);
			// TODO support pause for VOD
			if(_pListener->publication.flvReader)_pListener->publication.flvReader->paused(paused);
			if (paused) {
				// useless, client knows it when it calls NetStream::pause method
				writer.writeAMFStatus("NetStream.Pause.Notify", _pListener->publication.name() + " paused");
			} else {
				double position;
				if (message.readNumber(position))
					_pListener->seek((UInt32)position);
				//if (_pListener->publication.flvReader)_pListener->publication.flvReader->seek((UInt32)position);
				OnStart::raise(id, writer); // stream begin
				// useless, client knows it when it calls NetStream::resume method
				writer.writeAMFStatus("NetStream.Unpause.Notify", _pListener->publication.name() + " resumed");
			}
			return;
		}
		
		if (name == "seek") {
			double position;
			if (message.readNumber(position)) {
				_pListener->seek((UInt32)position);
				_pListener->publication.flvReader->seek((UInt32)position);
				OnStart::raise(id, writer); // stream begin
				// useless, client knows it when it calls NetStream::seek method, and wait "NetStream.Seek.Complete" rather (raised by client side)
				writer.writeAMFStatus("NetStream.Seek.Notify", _pListener->publication.name() + " seek operation");
			} else
				writer.writeAMFStatus("NetStream.Seek.InvalidTime", _pListener->publication.name() + " seek operation must pass in argument a milliseconds position time");
			return;
		}
	}

	ERROR("Message '",name,"' unknown on stream ",id);
}

void FlashStream::dataHandler(DataReader& data, double lostRate) {
	if(!_pPublication) {
		ERROR("a data packet has been received on a no publishing stream ",id,", certainly a publication currently closing");
		return;
	}

	// necessary AMF0 here!
	if (data.packet.available()>3 && *data.packet.current() == AMF_STRING && *(data.packet.current() + 3) == '@' && *(data.packet.current() + 1) == 0) {

		if (data.packet.available()>15 && *(data.packet.current() + 2) == 13 && memcmp(data.packet.current() + 3, EXPAND("@setDataFrame"))==0) {
			// @setDataFrame
			data.next();
			_pPublication->writeProperties(data);
			return;
		}

		if (data.packet.available()>17 && *(data.packet.current() + 2) == 15 && memcmp(data.packet.current() + 3, EXPAND("@clearDataFrame"))==0) {
			// @clearDataFrame
			_pPublication->clearProperties();
			return;
		}

	}

	_pPublication->pushData(data,peer.ping(),lostRate);
}

void FlashStream::rawHandler(UInt16 type, PacketReader& packet, FlashWriter& writer) {
	if(type==0x0022) { // TODO Here we receive RTMFP flow sync signal, useless to support it!
		//TRACE("Sync ",id," : ",data.read32(),"/",data.read32());
		return;
	}
	ERROR("Raw message ",Format<UInt16>("%.4x",type)," unknown on stream ",id);
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
