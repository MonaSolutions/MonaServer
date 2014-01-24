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

#include "Mona/Listener.h"
#include "Mona/Publication.h"
#include "Mona/MediaCodec.h"
#include "Mona/Logs.h"


using namespace std;

namespace Mona {

Listener::Listener(Publication& publication,Client& client,Writer& writer,bool unbuffered) : _droppedFrames(0),_unbuffered(unbuffered),
	_writer(writer),publication(publication),_firstKeyFrame(false),receiveAudio(true),receiveVideo(true),client(client),
	_pAudioWriter(NULL),_pVideoWriter(NULL),_pDataWriter(NULL),_publicationNamePacket((const UInt8*)publication.name().c_str(),publication.name().size()),
	_time(0),_deltaTime(0),_addingTime(0),_bufferTime(0),_firstAudio(true),_firstVideo(true),_firstTime(true) {
}

Listener::~Listener() {
	// -1 indicate that it come of the listener class
	if(_pAudioWriter)
		_pAudioWriter->close(-1);
	if(_pVideoWriter)
		_pVideoWriter->close(-1);
	if(_pDataWriter)
		_pDataWriter->close(-1);
}

void Listener::close(Writer& writer,int code){
	if (code == -1) // code == -1 when it's closing by itself
		return;
	// call by the possible WriterHandler
	// We have to reset the listener, and reset complelty the related writer (newWriter)
	if (_pVideoWriter == &writer)
		_pVideoWriter = NULL;
	else if (_pAudioWriter == &writer)
		_pAudioWriter = NULL;
	else if (_pDataWriter == &writer)
		_pDataWriter = NULL;
	init();
}

const QualityOfService& Listener::audioQOS() const {
	if(!_pAudioWriter)
		return QualityOfService::Null;
	return _pAudioWriter->qos();
}

const QualityOfService& Listener::videoQOS() const {
	if(!_pVideoWriter)
		return QualityOfService::Null;
	return _pVideoWriter->qos();
}

const QualityOfService& Listener::dataQOS() const {
	if(!_pDataWriter)
		return QualityOfService::Null;
	return _pDataWriter->qos();
}

bool Listener::init() {
	if (_pAudioWriter)
		WARN("Reinitialisation of one ",publication.name()," subscription")
	if (!_writer.writeMedia(Writer::INIT, 0, publicationNamePacket()))
		return false; // Here consider that the listener have to be closed by the caller
	init(&_pAudioWriter, Writer::AUDIO);
	init(&_pVideoWriter,Writer::VIDEO);
	init(&_pDataWriter,Writer::DATA);
	_time = 0;
	_addingTime = 0;
	_ts.update();
	_firstTime = true;
	return true;
}

void Listener::init(Writer** ppWriter,Writer::MediaType type) {
	if(*ppWriter == NULL) {
		*ppWriter = &_writer.newWriter(*this);
		if(_unbuffered)
			(*ppWriter)->reliable = false;
	}
	(*ppWriter)->writeMedia(Writer::INIT,type,publicationNamePacket());
}

UInt32 Listener::computeTime(UInt32 time) {
	if(time==0)
		time=(UInt32)(_ts.elapsed()/1000);
	if(_firstTime) { // first time, compute deltatime
		_deltaTime = time;
		_firstTime = false;
		DEBUG("Deltatime assignment, ",_deltaTime);
	}
	if(_deltaTime>time) {
		WARN("Subcription ",publication.name()," time ",time," inferior to deltaTime ",_deltaTime," (non increasing time)")
		_deltaTime = time;
	}
	_time = time-_deltaTime+_addingTime;
	TRACE("Time ",_time)
	return _time;
}

void Listener::startPublishing() {
	// publisher time will start to 0 here!
	_writer.writeMedia(Writer::START,0,publicationNamePacket());
	_firstKeyFrame=false;
	_ts.update();
}

void Listener::stopPublishing() {
	_deltaTime=0;
	_addingTime = _time;
	_droppedFrames = 0;
	_writer.writeMedia(Writer::STOP,0,publicationNamePacket());
}


void Listener::pushDataPacket(DataReader& reader) {
	if (!_pDataWriter && !init())
		return;

	if(publication.publisher()) {
		if(ICE::ProcessSDPPacket(reader,(Peer&)*publication.publisher(),publication.publisher()->writer(),(Peer&)client,*_pDataWriter))
			return;
	}

	if(!_pDataWriter->hasToConvert(reader)) {
		if(!_pDataWriter->writeMedia(Writer::DATA,0,reader.packet))
			init();
		return;
	}

	shared_ptr<DataWriter> pWriter;
	_pDataWriter->createWriter(pWriter);
	if (!pWriter) {
		if(!_pDataWriter->writeMedia(Writer::DATA,0,reader.packet))
			init();
		return;
	}
	
	UInt32 offset = pWriter->packet.size();
	reader.read(*pWriter);
	reader.reset();
	PacketReader packet(pWriter->packet.data(),pWriter->packet.size());
	packet.next(offset);
	if(!_pDataWriter->writeMedia(Writer::DATA,computeTime(0),packet))
		init();
}

void Listener::pushVideoPacket(PacketReader& packet,UInt32 time) {
	if(!receiveVideo) {
		_firstKeyFrame=false;
		_firstVideo=true;
		return;
	}
	if (!_pVideoWriter && !init())
		return;

	// key frame ?
	if(MediaCodec::IsKeyFrame(packet.current(),packet.available()))
		_firstKeyFrame=true;

	if(!_firstKeyFrame) {
		DEBUG("Video frame dropped to wait first key frame");
		++_droppedFrames;
		return;
	}

	time = computeTime(time);

	if(_firstVideo) {
		_firstVideo=false;
		UInt32 size(0);
		if(!MediaCodec::H264::IsCodecInfos(packet.current(),packet.available()) && (size=publication.videoCodecBuffer().size())>0) {
			PacketReader videoCodecPacket(publication.videoCodecBuffer().data(),size);
			// Reliable way for video codec packet!
			bool reliable = _pVideoWriter->reliable;
			_pVideoWriter->reliable = true;
			if (!_pVideoWriter->writeMedia(Writer::VIDEO, time, videoCodecPacket) && !init())
				return;
			_pVideoWriter->reliable = reliable;
		}
	}

	if(!_pVideoWriter->writeMedia(Writer::VIDEO,time,packet))
		init();
}


void Listener::pushAudioPacket(PacketReader& packet,UInt32 time) {
	if(!receiveAudio) {
		_firstAudio=true;
		return;
	}
	if (!_pAudioWriter && !init())
		return;

	time = computeTime(time);

	if(_firstAudio) {
		_firstAudio=false;
		UInt32 size(0);
		if(!MediaCodec::AAC::IsCodecInfos(packet.current(),packet.available()) && (size=publication.audioCodecBuffer().size())>0) {
			PacketReader audioCodecPacket(publication.audioCodecBuffer().data(),size);
			// Reliable way for audio codec packet!
			bool reliable = _pAudioWriter->reliable;
			_pAudioWriter->reliable = true;
			if (!_pAudioWriter->writeMedia(Writer::AUDIO, time, audioCodecPacket) && !init())
				return;
			_pAudioWriter->reliable = reliable;
		}
	}

	if(!_pAudioWriter->writeMedia(Writer::AUDIO,time,packet))
		init();
}

void Listener::flush() {
	if(_pAudioWriter)
		_pAudioWriter->flush();
	if(_pVideoWriter)
		_pVideoWriter->flush();
	if(_pDataWriter)
		_pDataWriter->flush();
	_writer.flush(true);
}


} // namespace Mona
