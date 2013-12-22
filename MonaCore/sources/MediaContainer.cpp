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

#include "Mona/MediaContainer.h"
#include "Mona/AMF.h"


using namespace std;

namespace Mona {

////////////////////  FLV  /////////////////////////////

// Write header
void MediaContainer::FLV::Write(BinaryWriter& writer,UInt8 track) {
	// just video		46 4c 56 01 01 00 00 00 09 00 00 00 00  
	// just audio		46 4c 56 01 04 00 00 00 09 00 00 00 00
	// audio and video	46 4c 56 01 05 00 00 00 09 00 00 00 00  
	UInt8 byteTrack = 0;
	if (track&VIDEO)
		byteTrack |= 1;
	if (track&AUDIO)
		byteTrack |= 4;
	writer.writeRaw("\x46\x4c\x56\x01",track,(const UInt8*)EXPAND_SIZE("\x00\x00\x00\x09\x00\x00\x00\x00"));
}

// Writer audio or video packet
void MediaContainer::FLV::Write(BinaryWriter& writer,UInt8 track,UInt32 time,const UInt8* data,UInt32 size) {
	/// 11 bytes of header
	writer.write8(track&AUDIO ? AMF::AUDIO : AMF::VIDEO);
	// size on 3 bytes
	writer.write24(size);
	// time on 3 bytes
	writer.write24(time);
	// unknown 4 bytes set to 0
	writer.write32(0);
	/// playload
	writer.writeRaw(data, size);
	/// footer
	writer.write32(11+size);
}

} // namespace Mona
