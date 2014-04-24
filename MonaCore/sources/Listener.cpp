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

Listener::Listener(Publication& publication,Client& client,Writer& writer) :
   _writer(writer), publication(publication), receiveAudio(true), receiveVideo(true), client(client), _firstTime(true),
	_pAudioWriter(NULL),_pVideoWriter(NULL),_pDataWriter(NULL),_publicationNamePacket((const UInt8*)publication.name().c_str(),publication.name().size()),
	_startTime(0),_codecInfosSent(false) {
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

void Listener::setBufferTime(UInt32 ms) {
	WARN("Listener::setBufferTime not implemented");
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
	if (_pVideoWriter || _pAudioWriter || _pDataWriter) {
		WARN("Reinitialisation of one ", publication.name(), " subscription");
		_pVideoWriter = _pAudioWriter = _pDataWriter = NULL;
	}
	if (!_writer.writeMedia(Writer::INIT, 0, publicationNamePacket(),*this))// unsubscribe can be done here!
		return false; // Here consider that the listener have to be closed by the caller

	_pAudioWriter = &_writer.newWriter();
	_pAudioWriter->writeMedia(Writer::INIT, Writer::AUDIO, publicationNamePacket(),*this);
	_pVideoWriter = &_writer.newWriter();
	_pVideoWriter->writeMedia(Writer::INIT, Writer::VIDEO, publicationNamePacket(),*this);
	_pDataWriter = &_writer.newWriter();
	_pDataWriter->writeMedia(Writer::INIT, Writer::DATA, publicationNamePacket(),*this);
	_pDataTypeName = typeid(*_pDataWriter).name();

	
	if(!publication.audioCodecBuffer().empty()) {
		PacketReader audioCodecPacket(publication.audioCodecBuffer()->data(),publication.audioCodecBuffer()->size());
		INFO("AAC codec infos sent to one listener of ", publication.name(), " publication")
		pushAudioPacket(_startTime,audioCodecPacket);
	}

	if (getBool<false>("unbuffered")) {
		_pAudioWriter->reliable = false;
		_pVideoWriter->reliable = false;
		_pDataWriter->reliable = false;
	}

	return true;
}

void Listener::pushDataPacket(DataReader& reader) {
	if (!_pDataWriter && !init())
		return;

	if(publication.publisher()) {
		if(ICE::ProcessSDPPacket(reader,(Peer&)*publication.publisher(),publication.publisher()->writer(),(Peer&)client,*_pDataWriter))
			return;
	}

	if(_pDataTypeName == typeid(reader).name()) {
		if(!_pDataWriter->writeMedia(Writer::DATA,0,reader.packet,*this))
			init();
		return;
	}

	shared_ptr<DataWriter> pWriter;
	_pDataWriter->createWriter(pWriter);
	if (!pWriter) {
		if(!_pDataWriter->writeMedia(Writer::DATA,0,reader.packet,*this))
			init();
		return;
	}
	
	UInt32 offset = pWriter->packet.size();
	reader.read(*pWriter);
	reader.reset();
	PacketReader packet(pWriter->packet.data(),pWriter->packet.size());
	packet.next(offset);
	if(!_pDataWriter->writeMedia(Writer::DATA,0,packet,*this))
		init();
}

void Listener::pushVideoPacket(UInt32 time,PacketReader& packet) {
	if(!receiveVideo && !MediaCodec::H264::IsCodecInfos(packet))
		return;

	if (!_codecInfosSent) {
		if (MediaCodec::IsKeyFrame(packet)) {
			if (!publication.videoCodecBuffer().empty() && !MediaCodec::H264::IsCodecInfos(packet)) {
				PacketReader videoCodecPacket(publication.videoCodecBuffer()->data(), publication.videoCodecBuffer()->size());
				INFO("H264 codec infos sent to one listener of ", publication.name(), " publication")
				pushVideoPacket(time, videoCodecPacket);
			}
			_codecInfosSent = true;
		} else if (_firstTime) {
			DEBUG("Video frame dropped to wait first key frame");
			return;
		}
	}
	

	if (!_pVideoWriter && !init())
		return;

	if (_firstTime) {
		_startTime = time;
		_firstTime = false;
	}
	time -= _startTime;

	TRACE("Video time ", time);

	if(!_pVideoWriter->writeMedia(Writer::VIDEO,time,packet,*this))
		init();
}


void Listener::pushAudioPacket(UInt32 time,PacketReader& packet) {
	if(!receiveAudio && !MediaCodec::AAC::IsCodecInfos(packet))
		return;

	if (!_pAudioWriter && !init())
		return;

	if (_firstTime) {
		_startTime = time;
		_firstTime = false;
	}
	time -= _startTime;

	TRACE("Audio time ", time);

	if(!_pAudioWriter->writeMedia(Writer::AUDIO,time,packet,*this))
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
