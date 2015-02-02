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

#include "Mona/WebSocket/WSDecoder.h"
#include "Mona/WebSocket/WS.h"

using namespace std;

namespace Mona {

UInt32 WSDecoder::decoding(Exception& ex, UInt8* data,UInt32 size) {
	BinaryReader reader(data, size);

	if (reader.available()<2)
		return 0;

	UInt8 type = reader.read8() & 0x0F;
	UInt8 lengthByte = reader.read8();

	size=lengthByte&0x7f;
	if (size==127) {
		if (reader.available()<8)
			return 0;
		size = (UInt32)reader.read64();
	} else if (size==126) {
		if (reader.available()<2)
			return 0;
		size = reader.read16();
	}

	if(lengthByte&0x80)
		size += 4;

	if (reader.available()<size)
		return 0;

	reader.shrink(size);

	if (lengthByte & 0x80)
		WS::Unmask(reader);

	receive(reader.current(),reader.available(),type);

	return reader.position()+reader.available();
}

} // namespace Mona
