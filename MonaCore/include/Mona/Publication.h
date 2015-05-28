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
#include "Mona/Listeners.h"
#include "Mona/Client.h"
#include "Mona/AMFWriter.h"

namespace Mona {

namespace PublicationEvents {
	struct OnData : Event<void(const Publication&,DataReader&)> {};
	struct OnAudio : Event<void(const Publication&,UInt32, PacketReader&)> {};
	struct OnVideo : Event<void(const Publication&,UInt32, PacketReader&)> {};
	struct OnFlush : Event<void(const Publication&)> {};
	struct OnProperties : Event<void(const Publication&,const Parameters&)> {};
};


class Publication : public virtual Object,
	public PublicationEvents::OnData,
	public PublicationEvents::OnAudio,
	public PublicationEvents::OnVideo,
	public PublicationEvents::OnFlush,
	public PublicationEvents::OnProperties {
public:
	enum Type {
		LIVE,
		RECORD
	};

	Publication(const std::string& name,const PoolBuffers& poolBuffers);
	virtual ~Publication();

	const PoolBuffers&		poolBuffers;

	const std::string&		name() const { return _name; }

	const Listeners			listeners;

	UInt64					droppedFrames() const { return _droppedFrames; }
	UInt32					lastTime() const { return _lastTime; }

	const QualityOfService&	videoQOS() const { return _videoQOS; }
	const QualityOfService&	audioQOS() const { return _audioQOS; }
	const QualityOfService&	dataQOS() const { return _dataQOS; }

	const Parameters&		properties() const { return _properties; }
	bool					requestProperties(Listener& listener) const;

	void					writeProperties(DataReader& reader);
	void					clearProperties();

	void					start(Type type);
	bool					running() const { return _running; }
	void					stop();

	void					pushAudio(UInt32 time,PacketReader& packet,UInt16 ping=0, double lostRate=0);
	void					pushVideo(UInt32 time,PacketReader& packet,UInt16 ping=0, double lostRate=0);
	void					pushData(DataReader& reader,UInt16 ping=0, double lostRate=0);

	Listener*				addListener(Exception& ex, Client& client,Writer& writer, const char* queryParameters=NULL);
	void					removeListener(Client& client);

	void					flush();

	
	const PoolBuffer&		audioCodecBuffer() const { return _audioCodecBuffer; }
	const PoolBuffer&		videoCodecBuffer() const { return _videoCodecBuffer; }

private:
	bool								_running;
	std::string							_name;
	std::map<Client*,Listener*>			_listeners;

	AMFWriter							_propertiesWriter; // In AMF because a format is required and AMF is a good network compressed format
	MapParameters						_properties;

	bool								_broken;
	UInt64								_droppedFrames;
	
	UInt32								_lastTime;
	PoolBuffer							_audioCodecBuffer;
	PoolBuffer							_videoCodecBuffer;

	QualityOfService					_videoQOS;
	QualityOfService					_audioQOS;
	QualityOfService					_dataQOS;

	bool								_new;
};


} // namespace Mona
