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

class MediaContainer : virtual Static {
public:
	enum Type {
		FLV,
		MPEG_TS
	};

	enum Track {
		AUDIO = 1,
		VIDEO = 2,
		BOTH = 3
	};

	template <typename ...Args>
	static void Write(Type type,BinaryWriter& writer, Args&&... args) {
		switch (type) {
			case FLV:
				FLV::Write(writer, args ...);
				break;
			case MPEG_TS:
				MPEGTS::Write(writer, args ...);
				break;
		}
		
	}
	
	class FLV : virtual Static {
	public:
		// To write header
		static void Write(BinaryWriter& writer,UInt8 track=BOTH);
		// To write audio or video packet
		static void Write(BinaryWriter& writer, UInt8 track, UInt32 time, const UInt8* data, UInt32 size);
	};

	class MPEGTS : virtual Static {
	public:
		// To write header
		static void Write(BinaryWriter& writer,UInt8 track=BOTH);
		// To write audio or video packet
		static void Write(BinaryWriter& writer, UInt8 track, UInt32 time, const UInt8* data, UInt32 size);
	private:

		static void	writeVideoPacket(BinaryWriter& writer, UInt32& available, UInt32 time, const UInt8* pData, bool isMetadata, bool first);

		static UInt32	CounterRow;
		static UInt32	CounterFrame;
		static char		CounterA;
	};

};




} // namespace Mona
