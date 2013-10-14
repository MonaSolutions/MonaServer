/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
class Listener {
public:
	Listener(Publication& publication,Client& client,Writer& writer,bool unbuffered);
	virtual ~Listener();

	void startPublishing();
	void stopPublishing(); 

	void pushAudioPacket(MemoryReader& packet,Mona::UInt32 time=0); 
	void pushVideoPacket(MemoryReader& packet,Mona::UInt32 time=0);
	void pushDataPacket(DataReader& packet);

	void flush();

	bool receiveAudio;
	bool receiveVideo;

	const Publication&	publication;
	Client&				client;

	Mona::UInt32			droppedFrames() const;
	const QualityOfService&	videoQOS() const;
	const QualityOfService&	audioQOS() const;
	const QualityOfService&	dataQOS() const;

	
	void setBufferTime(Mona::UInt32 ms);

private:
	void			init();
	void			init(Writer** ppWriter,Writer::MediaType type);
	Mona::UInt32 	computeTime(Mona::UInt32 time);

	bool					_unbuffered;
	bool					_firstKeyFrame;

	Mona::UInt32 			_deltaTime;
	Mona::UInt32 			_addingTime;
	Mona::UInt32 			_time;
	Mona::UInt32			_bufferTime;
	Mona::Time			_ts;
	
	Writer&					_writer;
	Writer*					_pAudioWriter;
	Writer*					_pVideoWriter;
	Writer*					_pDataWriter;
	Mona::UInt32			_droppedFrames;
	MemoryReader			_publicationReader;
};


inline void Listener::setBufferTime(Mona::UInt32 ms) {
	_bufferTime=ms;
}

inline Mona::UInt32 Listener::droppedFrames() const {
	return _droppedFrames;
}


} // namespace Mona
