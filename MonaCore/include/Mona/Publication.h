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
#include "Mona/Exceptions.h"
#include "Mona/Listeners.h"
#include "Mona/Peer.h"

namespace Mona {

class Publication : virtual Object {
public:
	Publication(const std::string& name);
	virtual ~Publication();

	const std::string&		name() const { return _name; }

	Client*					publisher() const { return _pPublisher; }

	const Listeners			listeners;

	UInt32					droppedFrames() const { return _droppedFrames; }

	const QualityOfService&	videoQOS() const { return _videoQOS; }
	const QualityOfService&	audioQOS() const { return _audioQOS; }
	const QualityOfService&	dataQOS() const { return _dataQOS; }

	void					setBufferTime(UInt32 ms);

	void					start(Exception& ex, Peer& peer);
	void					stop(Peer& peer);

	void					pushAudio(PacketReader& packet,UInt32 time=0,UInt32 numberLostFragments=0);
	void					pushVideo(PacketReader& packet,UInt32 time=0,UInt32 numberLostFragments=0);
	void					pushData(DataReader& reader,UInt32 numberLostFragments=0);

	Listener*				addListener(Exception& ex, Peer& peer,Writer& writer,bool unbuffered);
	void					removeListener(Peer& peer);

	void					flush();

	const Buffer&			audioCodecBuffer() const { return _audioCodecBuffer; }
	const Buffer&			videoCodecBuffer() const { return _videoCodecBuffer; }
private:
	Peer*								_pPublisher;
	bool								_firstKeyFrame;
	std::string							_name;
	std::map<Client*,Listener*>			_listeners;

	Buffer								_audioCodecBuffer;
	Buffer								_videoCodecBuffer;

	QualityOfService					_videoQOS;
	QualityOfService					_audioQOS;
	QualityOfService					_dataQOS;

	UInt32						_droppedFrames;
	bool						_new;
};


} // namespace Mona
