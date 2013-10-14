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
#include "Mona/Listeners.h"
#include "Mona/Peer.h"

namespace Mona {

class Publication {
public:
	Publication(const std::string& name);
	virtual ~Publication();

	const std::string&		name() const;

	Client*					publisher() const;

	const Listeners			listeners;

	Mona::UInt32			droppedFrames() const;

	const QualityOfService&	videoQOS() const;
	const QualityOfService&	audioQOS() const;
	const QualityOfService&	dataQOS() const;

	void					setBufferTime(Mona::UInt32 ms);

	void					start(Peer& peer);
	void					stop(Peer& peer);

	void					pushAudio(MemoryReader& packet,Mona::UInt32 time=0,Mona::UInt32 numberLostFragments=0);
	void					pushVideo(MemoryReader& packet,Mona::UInt32 time=0,Mona::UInt32 numberLostFragments=0);
	void					pushData(DataReader& data,Mona::UInt32 numberLostFragments=0);

	Listener&				addListener(Peer& peer,Writer& writer,bool unbuffered);
	void					removeListener(Peer& peer);

	void					flush();
private:
	Peer*								_pPublisher;
	bool								_firstKeyFrame;
	std::string							_name;
	std::map<Client*,Listener*>			_listeners;

	QualityOfService					_videoQOS;
	QualityOfService					_audioQOS;
	QualityOfService					_dataQOS;

	Mona::UInt32						_droppedFrames;
};

inline Mona::UInt32 Publication::droppedFrames() const {
	return _droppedFrames;
}

inline Client* Publication::publisher() const {
	return _pPublisher;
}

inline const QualityOfService& Publication::audioQOS() const {
	return _audioQOS;
}

inline const QualityOfService& Publication::videoQOS() const {
	return _videoQOS;
}

inline const QualityOfService& Publication::dataQOS() const {
	return _dataQOS;
}

inline const std::string& Publication::name() const {
	return _name;
}

} // namespace Mona
