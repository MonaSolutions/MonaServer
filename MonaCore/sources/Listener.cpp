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
#include "Mona/Logs.h"


using namespace std;

namespace Mona {

Listener::Listener(Publication& publication,Client& client,Writer& writer,bool unbuffered) : _droppedFrames(0),_unbuffered(unbuffered),
	_writer(writer),publication(publication),_firstKeyFrame(false),receiveAudio(true),receiveVideo(true),client(client),
	_pAudioWriter(NULL),_pVideoWriter(NULL),_pDataWriter(NULL),_publicationReader((const UInt8*)publication.name().c_str(),publication.name().size()),
	_time(0),_deltaTime(0),_addingTime(0),_bufferTime(0),_firstAudio(true),_firstVideo(true),_firstTime(true) {
}

Listener::~Listener() {
	if(_pAudioWriter)
		_pAudioWriter->close();
	if(_pVideoWriter)
		_pVideoWriter->close();
	if(_pDataWriter)
		_pDataWriter->close();
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

void Listener::init() {
	if(_pAudioWriter)
		WARN("Reinitialisation of one ",publication.name()," subscription")
	_writer.writeMedia(Writer::INIT,0,_publicationReader);
	init(&_pAudioWriter,Writer::AUDIO);
	init(&_pVideoWriter,Writer::VIDEO);
	init(&_pDataWriter,Writer::DATA);
	_time = 0;
	_addingTime = 0;
}

void Listener::init(Writer** ppWriter,Writer::MediaType type) {
	if(*ppWriter == NULL) {
		*ppWriter = &_writer.newWriter();
		if(_unbuffered)
			(*ppWriter)->reliable = false;
	}
	(*ppWriter)->writeMedia(Writer::INIT,type,_publicationReader);
}

UInt32 Listener::computeTime(UInt32 time) {
	if(_firstTime) { // has been initialized, compute deltatime
		_deltaTime = time;
		_firstTime = false;
		DEBUG("Deltatime assignment, ",_deltaTime);
	} else if(time==0)
		time=(UInt32)(_ts.elapsed()/1000);
	_ts.update();
	if(_deltaTime>time) {
		WARN("Subcription ",publication.name()," time ",time," inferior to deltaTime ",_deltaTime," (non increasing time)")
		_deltaTime = time;
	}
	_time = time-_deltaTime+_addingTime;
	TRACE("Time ",_time)
	return (_time = time);
}

void Listener::startPublishing() {
	_writer.writeMedia(Writer::START,0,_publicationReader);
	_firstKeyFrame=false;
	_ts.update();
}

void Listener::stopPublishing() {
	_firstTime = 0;
	_deltaTime=0;
	_addingTime = _time;
	_droppedFrames = 0;
	_writer.writeMedia(Writer::STOP,0,_publicationReader);
}


void Listener::pushDataPacket(DataReader& packet) {
	if(!_pDataWriter)
		init();

	if(publication.publisher()) {
		if(ICE::ProcessSDPPacket(packet,(Peer&)*publication.publisher(),publication.publisher()->writer(),(Peer&)client,*_pDataWriter))
			return;
	}

	if(!_pDataWriter->hasToConvert(packet)) {
		if(!_pDataWriter->writeMedia(Writer::DATA,0,packet.reader))
			init();
		return;
	}

	shared_ptr<DataWriter> pWriter;
	_pDataWriter->createWriter(pWriter);
	if (!pWriter) {
		if(!_pDataWriter->writeMedia(Writer::DATA,0,packet.reader))
			init();
		return;
	}
	
	UInt32 offset = pWriter->stream.size();
	packet.read(*pWriter);
	packet.reset();
	MemoryReader reader(pWriter->stream.data(),pWriter->stream.size());
	reader.next(offset);
	if(!_pDataWriter->writeMedia(Writer::DATA,0,reader))
		init();
}

void Listener::pushVideoPacket(MemoryReader& packet,UInt32 time) {
	if(!receiveVideo) {
		_firstKeyFrame=false;
		_firstVideo=true;
		return;
	}
	if(!_pVideoWriter)
		init();

	// key frame ?
	if(((*packet.current())&0xF0) == 0x10)
		_firstKeyFrame=true;

	if(!_firstKeyFrame) {
		DEBUG("Video frame dropped to wait first key frame");
		++_droppedFrames;
		return;
	}

	time = computeTime(time);

	if(_firstVideo) {
		_firstVideo=false;
		UInt32 size = publication.videoCodecBuffer().size();
		if(size>0) {
			MemoryReader videoCodecPacket(&publication.videoCodecBuffer()[0],size);
			// Reliable way for video codec packet!
			bool reliable = _pVideoWriter->reliable;
			_pVideoWriter->reliable = true;
			if (!_pVideoWriter->writeMedia(Writer::VIDEO, time, videoCodecPacket))
				init();
			_pVideoWriter->reliable = reliable;
		}
	}


	if(!_pVideoWriter->writeMedia(Writer::VIDEO,time,packet))
		init();
}


void Listener::pushAudioPacket(MemoryReader& packet,UInt32 time) {
	if(!receiveAudio) {
		_firstAudio=true;
		return;
	}
	if(!_pAudioWriter)
		init();

	time = computeTime(time);

	if(_firstAudio) {
		_firstAudio=false;
		UInt32 size = publication.audioCodecBuffer().size();
		if(size>0) {
			MemoryReader audioCodecPacket(&publication.audioCodecBuffer()[0],size);
			// Reliable way for audio codec packet!
			bool reliable = _pAudioWriter->reliable;
			_pAudioWriter->reliable = true;
			if(!_pAudioWriter->writeMedia(Writer::AUDIO,time,audioCodecPacket))
				init();
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
