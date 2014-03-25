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
#include "Mona/BinaryWriter.h"


namespace Mona {

#define MPEGTS_PACKET_SIZE		188
#define TS_PROGRAM_ID_AUDIO		0x60
#define TS_PROGRAM_ID_VIDEO		0x40

class SubstreamMap;

class MediaContainer : public virtual Object {
public:

	enum Track {
		AUDIO = 1,
		VIDEO = 2,
		BOTH = 3
	};

	// Write header
	virtual void write(BinaryWriter& writer,UInt8 track=BOTH) = 0;

	// Write audio or video packet
	virtual void write(BinaryWriter& writer, UInt8 track, UInt32 time, const UInt8* data, UInt32 size) = 0;
};

class FLV : public MediaContainer {
public:
	FLV(){}

	// To write header
	virtual void write(BinaryWriter& writer,UInt8 track=BOTH);
	// To write audio or video packet
	virtual void write(BinaryWriter& writer, UInt8 track, UInt32 time, const UInt8* data, UInt32 size);
};

class MPEGTS : public MediaContainer {
public:
	MPEGTS() {}

	// To write header
	virtual void write(BinaryWriter& writer,UInt8 track=BOTH);
	// To write audio or video packet
	virtual void write(BinaryWriter& writer, UInt8 track, UInt32 time, const UInt8* data, UInt32 size);
private:
	
	/// \brief Write recursively data of subReader in TS format
	void		writeTS(BinaryWriter& writer, UInt32& available, UInt32 time, SubstreamMap& subReader, bool isMetadata, Track type, bool first);

	/// \brief Parse each NALU (Video)
	/// Manages 2 types of NALU header :
	/// - 2 bytes header = 0x00XY (where XY = size of NALU)
	/// - 4 bytes header = 0x00UVWXYZ4/6 to 0x00UVWXYZ4/6 (where UVWXYZ = size of NALU)
	/// \return total size available
	static UInt32		ParseNAL(SubstreamMap& reader, const UInt8* data, UInt32 size);

	/// \brief Parse Audio frame
	/// \return size available
	static UInt32		ParseAudio(SubstreamMap& reader, const UInt8* data, UInt32 size);

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
