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
#include "Mona/BinaryReader.h"


namespace Mona {


class MediaCodec : virtual Static {
public:

	static bool IsKeyFrame(BinaryReader& reader) { return reader.available()>0 && (*reader.current()&0xF0)==0x10; }
	
	class H264 : virtual Static {
	public:
		// To write header
		static bool IsCodecInfos(BinaryReader& reader) { return reader.available()>1 && *reader.current() == 0x17 && reader.current()[1] == 0; }

	};

	class AAC : virtual Static {
	public:
		// To write header
		static bool IsCodecInfos(BinaryReader& reader) { return reader.available()>1 && (*reader.current() >> 4) == 0x0A && reader.current()[1] == 0; }

	};

};




} // namespace Mona
