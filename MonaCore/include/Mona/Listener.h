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

#pragma once

#include "Mona/Mona.h"
#include "Mona/Writer.h"
#include "Mona/QualityOfService.h"
#include "Mona/Client.h"

namespace Mona {

class Publication;
class Listener : virtual Object, WriterHandler {
public:
	Listener(Publication& publication,Client& client,Writer& writer,bool unbuffered);
	virtual ~Listener();

	void startPublishing();
	void stopPublishing(); 

	void pushAudioPacket(MemoryReader& packet,UInt32 time=0); 
	void pushVideoPacket(MemoryReader& packet,UInt32 time=0);
	void pushDataPacket(DataReader& packet);

	void flush();

	bool receiveAudio;
	bool receiveVideo;

	const Publication&	publication;
	Client&				client;

	UInt32					droppedFrames() const { return _droppedFrames; }
	const QualityOfService&	videoQOS() const;
	const QualityOfService&	audioQOS() const;
	const QualityOfService&	dataQOS() const;

	
	void setBufferTime(UInt32 ms) { _bufferTime = ms; }

private:
	void	init();
	void	init(Writer** ppWriter,Writer::MediaType type);
	UInt32 	computeTime(UInt32 time);

	/// WriterHandler implementation
	void	close(Writer& writer, int code);

	bool					_unbuffered;
	bool					_firstKeyFrame;
	bool					_firstAudio;
	bool					_firstVideo;
	bool					_firstTime;

	UInt32 					_deltaTime;
	UInt32 					_addingTime;
	UInt32 					_time;
	UInt32					_bufferTime;
	Time					_ts;
	
	Writer&					_writer;
	Writer*					_pAudioWriter;
	Writer*					_pVideoWriter;
	Writer*					_pDataWriter;
	UInt32					_droppedFrames;
	MemoryReader			_publicationReader;
};


} // namespace Mona
