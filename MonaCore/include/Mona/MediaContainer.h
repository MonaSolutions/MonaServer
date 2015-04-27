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
#include "Mona/MIME.h"
#include <map>


namespace Mona {

class SubstreamMap;

class MediaContainer : public virtual Object {
public:

	MediaContainer(const PoolBuffers& poolBuffers) : poolBuffers(poolBuffers) {}

	enum Track {
		AUDIO = 1,
		VIDEO = 2,
		BOTH = 3
	};

	const PoolBuffers& poolBuffers;

	// Write header
	virtual void write(BinaryWriter& writer,UInt8 track=BOTH) = 0;

	// Write audio or video packet
	virtual void write(BinaryWriter& writer, UInt8 track, UInt32 time, const UInt8* data, UInt32 size) = 0;

	// Write data (custom or metadata)
	virtual void write(BinaryWriter& writer, Writer::DataType dataType, MIME::Type packetType, PacketReader& packet);
};

class FLV : public MediaContainer {
public:
	FLV(const PoolBuffers& poolBuffers) : MediaContainer(poolBuffers) {}
	FLV(const Parameters& parameters, const PoolBuffers& poolBuffers);

	// To write header
	virtual void write(BinaryWriter& writer,UInt8 track=BOTH);
	// To write audio or video packet
	virtual void write(BinaryWriter& writer, UInt8 track, UInt32 time, const UInt8* data, UInt32 size);

	// To write data (custom or metadata)
	virtual void write(BinaryWriter& writer, Writer::DataType dataType, MIME::Type packetType, PacketReader& packet);
private:
	void writeFrame(BinaryWriter& writer, UInt8 type, UInt32 time, const UInt8* data, UInt32 size);

	std::string _onAudio;
	std::string _onVideo;
};

class RTP: public MediaContainer {
public:
	RTP(const PoolBuffers& poolBuffers, const char* ssrc): _counter(0), _SSRC(ssrc), _octetCount(0), MediaContainer(poolBuffers) {}
	
	// To write header (empty because there is no header in RTP)
	virtual void write(BinaryWriter& writer,UInt8 track=BOTH) {}
	// To write audio or video packet
	virtual void write(BinaryWriter& writer,UInt8 track,UInt32 time,const UInt8* data,UInt32 size);

	void writeRTCP(BinaryWriter& writer,UInt8 type,UInt32 time);

private:
	
	UInt16			_counter;
	UInt32			_octetCount;
	const char*		_SSRC;
};

class MPEGTS : public MediaContainer {
public:
	MPEGTS(const PoolBuffers& poolBuffers) : MediaContainer(poolBuffers){}

	// To write header
	virtual void write(BinaryWriter& writer,UInt8 track=BOTH);
	// To write audio or video packet
	virtual void write(BinaryWriter& writer, UInt8 track, UInt32 time, const UInt8* data, UInt32 size);

	/// \brief Parse each NALU (Video)
	/// Manages 2 types of NALU header :
	/// - 2 bytes header = 0x00XY (where XY = size of NALU)
	/// - 4 bytes header = 0x00UVWXYZ4/6 to 0x00UVWXYZ4/6 (where UVWXYZ = size of NALU)
	/// \return total size available
	static UInt32 ParseNAL(SubstreamMap& reader, const UInt8* data, UInt32 size);

	/// \brief Parse Audio frame
	/// \return size available
	static UInt32 ParseAudio(SubstreamMap& reader, const UInt8* data, UInt32 size);

private:
	
	/// \brief Write recursively data of subReader in TS format
	void		writeTS(BinaryWriter& writer, UInt32& available, UInt32 time, SubstreamMap& subReader, bool isMetadata, Track type, bool first);


	/// \brief Determine if adaptive field is needed and return size of adaptive field 
	/// \return return size of adaptive field
	static UInt8		GetAdaptiveSize(bool time, UInt32 available, bool first, bool& adaptiveField, Track type);

	/// \brief Write payload of TS
	/// \return false if format error detected
	static bool			WritePES(BinaryWriter& writer, Track type, SubstreamMap& subReader, UInt8& toWrite);

	/// \brief Determine CRC32 value of input data
	//static UInt32	CalcCrc32(UInt8 * data, UInt32 datalen);
	//static UInt32					CrcTab[];

	std::map<Track, UInt32>	_counterRow;			///< Counter for each program/track
};



} // namespace Mona
