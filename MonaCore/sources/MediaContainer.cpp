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
#include "Mona/Logs.h"

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
	writer.writeRaw("\x46\x4c\x56\x01");
	writer.write8(track);
	writer.writeRaw(EXPAND_DATA_SIZE("\x00\x00\x00\x09\x00\x00\x00\x00"));
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

////////////////////  MPEG_TS  /////////////////////////////

#define BeginBuff1		"\x47\x40\x00\x30\xa6\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\xb0\x0d\x00\x01\xc1\x00\x00\x00\x01\xe0\x20\xa2\xc3\x29\x41"

#define BeginBuff2		"\x47\x40\x20\x30\x8b\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x02\xb0\x28\x00\x01\xc1\x00\x00\xe0\x40\xf0\x0c\x05\x04\x48\x44\x4d\x56\x88\x04\x0f\xff\xfc\xfc\x1b\xe0\x40\xf0\x0a\x05\x08\x48\x44\x4d\x56\xff\x1b\x44\x3f\xfb\xa2\xe2\x49"

UInt32	MediaContainer::MPEGTS::CounterRow(0);
UInt32	MediaContainer::MPEGTS::CounterFrame(0);
char	MediaContainer::MPEGTS::CounterA(0);

// Write header
void MediaContainer::MPEGTS::Write(BinaryWriter& writer,UInt8 track) {
	
}

void normalize(const UInt8* data, UInt32 size) {

	MemoryWriter writerTmp((UInt8 *)data, size);

	if (*data==0x27 || *data==0x17 && *(data+9)==0x09) {
		
		UInt8* pos = (UInt8*)data+11;
		writerTmp.next(11);
		while(size-(pos-data) > 4) {

			if (*pos!=0x00)
				++pos;
			else if (*(pos+1)!=0x00)
				pos += 2;		
			else if (*(pos+2)>0x07) { // TODO? 0x000008 to 0x0000ff
				pos += 3;
			}
			else { // 0x000000 to 0x000007
				if (*(pos+2)!=0x00) {
					writerTmp.next((pos-data)-writerTmp.position()+2);
					writerTmp.write8(0x00);
				} else 
					writerTmp.next((pos-data)-writerTmp.position()+3);
				
				writerTmp.write8(0x01); // 0x00000001
				pos+=4;
			}
		}
	}
}

UInt8 getAdaptiveSize(bool time, UInt32 available, bool first, bool& adaptiveField) {
	
	UInt8 size = 0;
	UInt8 max = MPEGTS_PACKET_SIZE - 4 - (first*18);
	if (available < max) {

		adaptiveField = true;
		if (available < max-1)
			size += (max-1) - available;
	}

	if (first && time) {

		// Init Adaptive field needed?
		if (!size) {
			adaptiveField = true;
			size += 1;
		}
		size += 6; // Six Bytes of timecode
	}

	return size;
}

// Writer audio or video packet
void MediaContainer::MPEGTS::Write(BinaryWriter& writer,UInt8 track,UInt32 time,const UInt8* data,UInt32 size) {

	// Temporary
	normalize(data, size);

	if (track&VIDEO && size>13 && *(data+9)==0x09) {

		UInt32 available=size-9;
		while (available > 0) {

			writeVideoPacket(writer, available, time, data+(size-available), *data == 0x17, available == (size-9));
		}
	}
}

void MediaContainer::MPEGTS::writeVideoPacket(BinaryWriter& writer, UInt32& available, UInt32 time, const UInt8* pData, bool isMetadata, bool first) {

	INFO("Time : ", time)

	// TODO? Audio data each three video packets
	if (first && (CounterFrame % 3) == 0) {

		writer.write8(0x47);
		writer.write8(0x40);
		writer.write8(0x00);
		writer.write8(0x30 + (CounterA & 0x0F));
		writer.writeRaw((UInt8 *)&BeginBuff1[4], MPEGTS_PACKET_SIZE - 4);
					
		writer.write8(0x47);
		writer.write8(0x40);
		writer.write8(0x20);
		writer.write8(0x30 + (CounterA++ & 0x0F));
		writer.writeRaw((UInt8 *)&BeginBuff2[4], MPEGTS_PACKET_SIZE - 4);
	}
	
	// header
	writer.write8(0x47);
	writer.write8(first? 0x40 : 0x00);
	writer.write8(0x40);

	// middle raws
	UInt32 toWrite = available;
	if (!first && available > MPEGTS_PACKET_SIZE - 4) {

		writer.write8(0x10 + (CounterRow++ & 0x0F));  // adaptive field off + Id
		toWrite = MPEGTS_PACKET_SIZE - 4;
	} else {
		
		// Adaptive Field
		bool adaptiveField = false;
		bool timeCode = true;//(CounterFrame % 2) == 0;
		UInt8 adaptiveSize = getAdaptiveSize(timeCode, available, first, adaptiveField);
		UInt8 nbFFBytes = 0;
		toWrite = MPEGTS_PACKET_SIZE - 4 - adaptiveSize - adaptiveField - (first*18);

		UInt64 pts = time*90; // PTS is 90KHz time
			
		// PTS Time is on 5 bytes
		UInt8 ptsBytes[5];
		ptsBytes[0] = ((pts >> 29) & 0x06) + 0x21;
		ptsBytes[1] = (pts >> 22) & 0xFF;
		ptsBytes[2] = ((pts >> 14) & 0xFE) + 0x01;
		ptsBytes[3] = (pts >> 7) & 0xFF;
		ptsBytes[4] = (pts << 1) + 0x01;

		UInt32 ptsEncoded = ptsBytes[4] + (ptsBytes[3] << 8) + (ptsBytes[2] << 16) + (ptsBytes[1] << 24);

		// adaptive field marker + Id
		writer.write8((adaptiveField?0x30:0x10) + (CounterRow++ & 0x0F));
		
		if (adaptiveField) {

			// size of Adaptive Field
			writer.write8(adaptiveSize);

			if (adaptiveSize) {

				nbFFBytes = adaptiveSize - 1;

				// 6 bytes Timecode
				if (first && timeCode) {

					nbFFBytes -= 6;
					
					// Adaptive timecode flag on
					writer.write8(isMetadata? 0x50 : 0x10);


					writer.write32(ptsEncoded >> 3); // pcr from pts					
					writer.write8(0x00);
					writer.write8(0x00);
				}
				else
					writer.write8(0x00); // Adaptive timecode flag off
			}
		}

		// empty cells
		for (int i = 0; i < nbFFBytes; i++)
			writer.write8(0xFF);

		// Video header (18 bytes)
		if (first) {

			writer.writeRaw(EXPAND_DATA_SIZE("\x00\x00\x01\xe0\x00\x00\x81\x80\x05\x21")); // Marker init video
			//writer.writeRaw(EXPAND_DATA_SIZE("\x00\x00\x01\xe0\x00\x00\x81\x80\x05")); // Marker init video

			writer.write32(ptsEncoded);

			writer.write32(1); // 0x00 0x00 0x00 0x01
			CounterFrame++; // It is a new frame
		}
	}
			
	writer.writeRaw(pData, toWrite);
	available -= toWrite;
}

} // namespace Mona
