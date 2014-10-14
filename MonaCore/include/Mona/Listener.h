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
#include "Mona/MapParameters.h"
#include "Mona/DataReader.h"

namespace Mona {

class Publication;
class Listener : public MapParameters, virtual Object {
public:
	Listener(Publication& publication,Client& client,Writer& writer);
	virtual ~Listener();

	void startPublishing() { _startTime = 0; _firstTime = true; _writer.writeMedia(Writer::START, 0, publicationNamePacket(), *this); }
	void stopPublishing() { _writer.writeMedia(Writer::STOP, 0, publicationNamePacket(), *this); }

	void pushAudio(UInt32 time,PacketReader& packet); 
	void pushVideo(UInt32 time,PacketReader& packet);
	void pushData(DataReader& packet);
	void updateProperties();

	void flush();

	bool receiveAudio;
	bool receiveVideo;

	const Publication&	publication;
	Client&				client;

	const QualityOfService&	videoQOS() const;
	const QualityOfService&	audioQOS() const;
	const QualityOfService&	dataQOS() const;

private:
	void    writeData(DataReader& reader,Writer::DataType type, Writer& writer);

	bool	init();
	void	reset();
	void	firstMedia(UInt32 time);

	PacketReader& publicationNamePacket() { _publicationNamePacket.reset(); return _publicationNamePacket; }
	
	bool					_firstMedia;
	UInt32 					_startTime;
	bool					_firstTime;
	bool					_codecInfosSent;
	
	Writer&					_writer;
	Writer*					_pAudioWriter;
	Writer*					_pVideoWriter;
	Writer*					_pDataWriter;
	PacketReader				_publicationNamePacket;
};


} // namespace Mona
