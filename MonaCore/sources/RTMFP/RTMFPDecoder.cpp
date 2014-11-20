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

#include "Mona/RTMFP/RTMFPDecoder.h"
#include "Mona/Crypto.h"

using namespace std;

namespace Mona {

UInt32 RTMFPDecoder::decoding(Exception& ex, UInt8* data,UInt32 size) {
	// Decrypt
	_pDecoder->process(data, size);
	// Check CRC
	BinaryReader reader(data, size);
	UInt16 crc(reader.read16());
	if (Crypto::ComputeCRC(reader) == crc)
		receive(reader.current(),reader.available());
	else
		ex.set(Exception::CRYPTO, "Bad RTMFP CRC sum computing");	
	return size;
}

} // namespace Mona
