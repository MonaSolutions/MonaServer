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


class MediaCodec : virtual Static {
public:

	static bool IsKeyFrame(const UInt8* data, UInt32 size) { return size>0 && (*data&0xF0)==0x10; }
	
	class H264 : virtual Static {
	public:
		// To write header
		static bool IsCodecInfos(const UInt8* data, UInt32 size) { return size>1 && *data == 0x17 && data[1] == 0; }

	};

	class AAC : virtual Static {
	public:
		// To write header
		static bool IsCodecInfos(const UInt8* data, UInt32 size) { return size > 1 && (*data >> 4) == 0x0A && data[1] == 0; }

	};

};




} // namespace Mona
