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

Listener::Listener(Publication& publication, Client& client, Writer& writer, const char* queryParameters) : _writer(writer), publication(publication), receiveAudio(true), receiveVideo(true), client(client), _firstTime(true),
	_seekTime(0),_pAudioWriter(NULL),_pVideoWriter(NULL),_publicationNamePacket((const UInt8*)publication.name().c_str(),publication.name().size()),
	_dataInitialized(false),_reliable(true),_startTime(0),_lastTime(0),_codecInfosSent(false) {
	if (queryParameters)
		Util::UnpackQuery(queryParameters, *this);
}

Listener::~Listener() {
	closeWriters();
}

void Listener::onChange(const char* key, const char* value, std::size_t size) {
	if (String::ICompare(key, EXPAND("unbuffered")) == 0)
		_reliable = String::IsFalse(value, size);
	Parameters::onChange(key, value, size);
}

void Listener::closeWriters() {
	// -1 indicate that it come of the listener class
	if(_pAudioWriter)
		_pAudioWriter->close(-1);
	if(_pVideoWriter)
		_pVideoWriter->close(-1);
	_pVideoWriter = _pAudioWriter = NULL;
	_dataInitialized = false;
}

bool Listener::initWriters() {
	// if start return false, the subscriber must unsubcribe the listener (closed by the caller)

	bool firstTime(false);

	if (_pVideoWriter || _pAudioWriter || _dataInitialized) {
		closeWriters();
		WARN("Reinitialisation of one ", publication.name(), " subscription");
	} else
		firstTime = true;

	_dataInitialized = true;
	if (!writeReliableMedia(_writer,Writer::INIT, Writer::DATA, publicationNamePacket(),*this))// unsubscribe can be done here!
		return false; // Here consider that the listener have to be closed by the caller

	_pAudioWriter = &_writer.newWriter();
	if (!writeReliableMedia(*_pAudioWriter, Writer::INIT, Writer::AUDIO, publicationNamePacket(), *this)) {
		closeWriters();
		return false; // Here consider that the listener have to be closed by the caller
	}
	_pVideoWriter = &_writer.newWriter();
	if (!writeReliableMedia(*_pVideoWriter,Writer::INIT, Writer::VIDEO, publicationNamePacket(), *this)) {
		closeWriters();
		return false; // Here consider that the listener have to be closed by the caller
	}

	if (firstTime && publication.running()) {
		startPublishing();
		// send publication properties (metadata)
		publication.requestProperties(*this);
	}

	return true;
}


void Listener::startPublishing() {

	if (!_pVideoWriter || !_pAudioWriter || !_dataInitialized) {
		initWriters(); // call already recursivly startPublishing()!
		return;
	}

	if (!writeReliableMedia(_writer,Writer::START, Writer::DATA, publicationNamePacket(), *this))// unsubscribe can be done here!
		return;
	if (!writeReliableMedia(*_pAudioWriter, Writer::START, Writer::AUDIO, publicationNamePacket(), *this))
		return; // Here consider that the listener have to be closed by the caller
	if (!writeReliableMedia(*_pVideoWriter, Writer::START, Writer::VIDEO, publicationNamePacket(), *this))
		return; // Here consider that the listener have to be closed by the caller
}

void Listener::stopPublishing() {

	if (firstTime())
		return;

	if (!_pVideoWriter || !_pAudioWriter || !_dataInitialized) {
		if (!initWriters())
			return;
	}

	if (!writeReliableMedia(_writer, Writer::STOP, Writer::DATA, publicationNamePacket(), *this))// unsubscribe can be done here!
		return;
	if (!writeReliableMedia(*_pAudioWriter, Writer::STOP, Writer::AUDIO, publicationNamePacket(), *this))
		return; // Here consider that the listener have to be closed by the caller
	if (!writeReliableMedia(*_pVideoWriter, Writer::STOP, Writer::VIDEO, publicationNamePacket(), *this))
		return; // Here consider that the listener have to be closed by the caller

	_seekTime = _lastTime;
	_firstTime = true;
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

	writeData(reader, Writer::DATA_USER);
}

void Listener::writeData(DataReader& reader,Writer::DataType type) {
	if (!_dataInitialized && !initWriters())
		return;

	if (!reader) {
		ERROR("Impossible to stream ", typeid(reader).name(), " null DataReader");
		return;
	}

	if (!writeMedia(_writer, type == Writer::DATA_INFO || _reliable, Writer::DATA, MIME::DataType(reader) << 8 | type, reader.packet, *this))
		initWriters();
}

void Listener::pushVideo(UInt32 time,PacketReader& packet) {
	if(!receiveVideo && !MediaCodec::H264::IsCodecInfos(packet))
		return;

	if (!_codecInfosSent) {
		if (MediaCodec::IsKeyFrame(packet)) {
			_codecInfosSent = true;
			if (!publication.videoCodecBuffer().empty() && !MediaCodec::H264::IsCodecInfos(packet)) {
				PacketReader videoCodecPacket(publication.videoCodecBuffer()->data(), publication.videoCodecBuffer()->size());
				INFO("H264 codec infos sent to one listener of ", publication.name(), " publication")
				pushVideo(time, videoCodecPacket);
			}
		} else {
			DEBUG("Video frame dropped to wait first key frame");
			return;
		}
	}

	if (!_pVideoWriter && !initWriters())
		return;

	if (_firstTime) {
		_startTime = time;
		_firstTime = false;

		// for audio sync (audio is usually the reference track)
		if (pushAudioInfos(time))
			pushAudio(time, PacketReader::Null); // push a empty audio packet to avoid a video which waits audio tracks!
	}
	time -= _startTime;

	TRACE("Video time(+seekTime) => ", time, "(+", _seekTime, ") ", Util::FormatHex(packet.current(), 5, LOG_BUFFER));

	if(!writeMedia(*_pVideoWriter, MediaCodec::IsKeyFrame(packet) || _reliable, Writer::VIDEO, _lastTime=(time+_seekTime), packet, *this))
		initWriters();
}


void Listener::pushAudio(UInt32 time,PacketReader& packet) {
	if(!receiveAudio && !MediaCodec::AAC::IsCodecInfos(packet))
		return;

	if (!_pAudioWriter && !initWriters())
		return;

	if (_firstTime) {
		_firstTime = false;
		_startTime = time;
		pushAudioInfos(time);
	}
	time -= _startTime;

	TRACE("Audio time(+seekTime) => ", time,"(+",_seekTime,")");

	if(!writeMedia(*_pAudioWriter, MediaCodec::AAC::IsCodecInfos(packet) || _reliable, Writer::AUDIO, _lastTime=(time+_seekTime), packet, *this))
		initWriters();
}

void Listener::pushProperties(DataReader& packet) {
	INFO("Properties sent to one listener of ",publication.name()," publication")
	writeData(packet,Writer::DATA_INFO);
}

bool Listener::pushAudioInfos(UInt32 time) {
	if (publication.audioCodecBuffer().empty())
		return false;
	PacketReader audioCodecPacket(publication.audioCodecBuffer()->data(), publication.audioCodecBuffer()->size());
	INFO("AAC codec infos sent to one listener of ", publication.name(), " publication")
	pushAudio(time, audioCodecPacket);
	return true;
}

void Listener::flush() {
	// in first data channel
	_writer.flush();
	// now media channel
	if(_pAudioWriter) // keep in first, because audio track is sometimes the time reference track
		_pAudioWriter->flush();
	if(_pVideoWriter)
		_pVideoWriter->flush();
}

bool Listener::writeMedia(Writer& writer, bool reliable, Writer::MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties) {
	bool wasReliable(writer.reliable);
	writer.reliable = reliable;
	bool success(writer.writeMedia(type,time,packet,properties));
	writer.reliable = wasReliable;
	return success;
}

} // namespace Mona
