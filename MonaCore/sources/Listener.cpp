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
#include "Mona/MIME.h"
#include "Mona/Logs.h"


using namespace std;

namespace Mona {

Listener::Listener(Publication& publication, Client& client, Writer& writer) : _writer(writer), publication(publication), receiveAudio(true), receiveVideo(true), client(client), _firstTime(true),
	_seekTime(0),_pAudioWriter(NULL),_pVideoWriter(NULL),_publicationNamePacket((const UInt8*)publication.name().c_str(),publication.name().size()),
	_dataInfos(DATA_NONE),_startTime(0),_lastTime(0),_codecInfosSent(false),_firstMedia(true) {
}

Listener::~Listener() {
	closeWriters();
}

void Listener::closeWriters() {
	// -1 indicate that it come of the listener class
	if(_pAudioWriter)
		_pAudioWriter->close(-1);
	if(_pVideoWriter)
		_pVideoWriter->close(-1);
	_pVideoWriter = _pAudioWriter = NULL;
}

bool Listener::initWriters() {
	// if start return false, the subscriber must unsubcribe the listener (closed by the caller)

	if (_pVideoWriter || _pAudioWriter || _dataInfos&DATA_INITIALIZED) {
		closeWriters();
		WARN("Reinitialisation of one ", publication.name(), " subscription");
	}
	if (!_writer.writeMedia(Writer::INIT, Writer::DATA, publicationNamePacket(),*this))// unsubscribe can be done here!
		return false; // Here consider that the listener have to be closed by the caller

	_dataInfos = DATA_INITIALIZED;
	if (_writer.reliable)
		_dataInfos |= DATA_WASRELIABLE;

	_pAudioWriter = &_writer.newWriter();
	if (!_pAudioWriter->writeMedia(Writer::INIT, Writer::AUDIO, publicationNamePacket(), *this)) {
		closeWriters();
		return false; // Here consider that the listener have to be closed by the caller
	}
	_pVideoWriter = &_writer.newWriter();
	if (!_pVideoWriter->writeMedia(Writer::INIT, Writer::VIDEO, publicationNamePacket(), *this)) {
		closeWriters();
		return false; // Here consider that the listener have to be closed by the caller
	}

	if (getBool<false>("unbuffered")) {
		_pAudioWriter->reliable = false;
		_pVideoWriter->reliable = false;
	} else
		_dataInfos |= DATA_RELIABLE;

	return true;
}


void Listener::startPublishing() {

	if (!_pVideoWriter || !_pAudioWriter || !(_dataInfos&DATA_INITIALIZED)) {
		if (!initWriters())
			return;
	}

	if (!_writer.writeMedia(Writer::START, Writer::DATA, publicationNamePacket(), *this))// unsubscribe can be done here!
		return;
	if (!_pAudioWriter->writeMedia(Writer::START, Writer::AUDIO, publicationNamePacket(), *this))
		return; // Here consider that the listener have to be closed by the caller
	if (!_pVideoWriter->writeMedia(Writer::START, Writer::VIDEO, publicationNamePacket(), *this))
		return; // Here consider that the listener have to be closed by the caller
}

void Listener::stopPublishing() {

	if (!_pVideoWriter || !_pAudioWriter || !(_dataInfos&DATA_INITIALIZED)) {
		if (!initWriters())
			return;
	}

	if (!_writer.writeMedia(Writer::STOP, Writer::DATA, publicationNamePacket(), *this))// unsubscribe can be done here!
		return;
	if (!_pAudioWriter->writeMedia(Writer::STOP, Writer::AUDIO, publicationNamePacket(), *this))
		return; // Here consider that the listener have to be closed by the caller
	if (!_pVideoWriter->writeMedia(Writer::STOP, Writer::VIDEO, publicationNamePacket(), *this))
		return; // Here consider that the listener have to be closed by the caller

	_seekTime = _lastTime;
	_firstTime = _firstMedia = true;
	_codecInfosSent = false;
	_startTime = 0;
}

void Listener::seek(UInt32 time) {
	// To force the time to be as requested, but the stream continue normally (not reseted _codecInfosSent and _firstMedia)
	_firstTime = true;
	_startTime = 0;
	_lastTime = _seekTime = time;
	NOTE("NEW SEEK_TIME = ",_seekTime);
}


void Listener::pushData(DataReader& reader) {
	/* TODO remplacer par un relay mode à imaginer et concevoir!
	if(publication.publisher()) {
		if(ICE::ProcessSDPPacket(reader,(Peer&)*publication.publisher(),publication.publisher()->writer(),(Peer&)client,*_pDataWriter))
			return;
	}*/

	writeData(reader, Writer::USER_DATA);
}

void Listener::writeData(DataReader& reader,Writer::DataType type) {
	if (!(_dataInfos&DATA_INITIALIZED) && !initWriters())
		return;

	if (!reader) {
		ERROR("Impossible to stream ", typeid(reader).name(), " null DataReader");
		return;
	}
	_writer.reliable = _dataInfos&DATA_RELIABLE ? true : false;
	bool success(_writer.writeMedia(Writer::DATA, MIME::DataType(reader) << 8 | type, reader.packet, *this));
	_writer.reliable = _dataInfos&DATA_WASRELIABLE ? true : false;
	if(!success)
		initWriters();
}

void Listener::pushVideo(UInt32 time,PacketReader& packet) {
	if(!receiveVideo && !MediaCodec::H264::IsCodecInfos(packet))
		return;

	firstMedia(time);

	if (!_codecInfosSent) {
		if (MediaCodec::IsKeyFrame(packet)) {
			_codecInfosSent = true;
			if (!publication.videoCodecBuffer().empty() && !MediaCodec::H264::IsCodecInfos(packet)) {
				PacketReader videoCodecPacket(publication.videoCodecBuffer()->data(), publication.videoCodecBuffer()->size());
				INFO("H264 codec infos sent to one listener of ", publication.name(), " publication")
				pushVideo(time, videoCodecPacket);
			}
		} else if (_firstTime) {
			DEBUG("Video frame dropped to wait first key frame");
			return;
		}
	}

	if (!_pVideoWriter && !initWriters())
		return;

	if (_firstTime) {
		_startTime = time;
		_firstTime = false;
	}
	time -= _startTime;

	TRACE("Video time(+seekTime) => ", time, "(+", _seekTime, ") ", Util::FormatHex(packet.current(), 5, LOG_BUFFER));

	if(!_pVideoWriter->writeMedia(Writer::VIDEO, _lastTime=(time+_seekTime), packet, *this))
		initWriters();
}


void Listener::pushAudio(UInt32 time,PacketReader& packet) {
	if(!receiveAudio && !MediaCodec::AAC::IsCodecInfos(packet))
		return;

	if (!_pAudioWriter && !initWriters())
		return;

	firstMedia(time);

	if (_firstTime) {
		_firstTime = false;
		_startTime = time;
	}
	time -= _startTime;

	TRACE("Audio time(+seekTime) => ", time,"(+",_seekTime,")");
	
	if (!_pAudioWriter->writeMedia(Writer::AUDIO, _lastTime=(time+_seekTime), packet, *this))
		initWriters();
}

void Listener::pushProperties(DataReader& packet) {
	INFO("Properties sent to one listener of ",publication.name()," publicaition")
	writeData(packet,Writer::INFO_DATA);
}

void Listener::firstMedia(UInt32 time) {
	if (!_firstMedia)
		return;
	_firstMedia = false; // keep it here cause recursive call

	if(!publication.audioCodecBuffer().empty()) {
		PacketReader audioCodecPacket(publication.audioCodecBuffer()->data(), publication.audioCodecBuffer()->size());
		INFO("AAC codec infos sent to one listener of ", publication.name(), " publication")
		pushAudio(time, audioCodecPacket);
	} else {
		// push a empty audio packet to avoid a video which waits audio tracks!
		pushAudio(time, PacketReader::Null);
	}
}

void Listener::flush() {
	// in first data channel
	_writer.flush();
	// now media channel
	if(_pAudioWriter)
		_pAudioWriter->flush();
	if(_pVideoWriter)
		_pVideoWriter->flush();
}


} // namespace Mona
