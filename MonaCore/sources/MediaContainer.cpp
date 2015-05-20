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

#include "Mona/MediaContainer.h"
#include "Mona/MediaCodec.h"
#include "Mona/AMF.h"
#include "Mona/AMFWriter.h"
#include "Mona/Logs.h"
#include "Mona/SubstreamMap.h"
#include <algorithm>

using namespace std;

namespace Mona {

////////////////////  BASE  ////////////////////////////

void MediaContainer::write(BinaryWriter& writer, Writer::DataType dataType, MIME::Type packetType, PacketReader& packet) {
	WARN(typeid(*this).name()," muxer doesn't support data writing operation")
}

////////////////////  FLV  /////////////////////////////

FLV::FLV(const Parameters& parameters, const PoolBuffers& poolBuffers) : MediaContainer(poolBuffers) {
	parameters.getString("onAudio", _onAudio);
	parameters.getString("onVideo", _onVideo);
}

// Write header
void FLV::write(BinaryWriter& writer,UInt8 track) {
	// just video		46 4c 56 01 01 00 00 00 09 00 00 00 00  
	// just audio		46 4c 56 01 04 00 00 00 09 00 00 00 00
	// audio and video	46 4c 56 01 05 00 00 00 09 00 00 00 00  
	UInt8 byteTrack = 0;
	if (track&VIDEO)
		byteTrack |= 1;
	if (track&AUDIO)
		byteTrack |= 4;
	writer.write(EXPAND("\x46\x4c\x56\x01")).write8(byteTrack).write(EXPAND("\x00\x00\x00\x09\x00\x00\x00\x00"));
}

// Write audio or video packet
void FLV::write(BinaryWriter& writer, UInt8 track, UInt32 time, const UInt8* data, UInt32 size) {
	if ((track&AUDIO && !_onAudio.empty()) || !_onVideo.empty()) {
		 // TODO MONASERVER2 => write directly on BinaryWriter
		AMFWriter amf(poolBuffers);
		amf.amf0 = true;
		if (track&AUDIO)
			amf.writeString(_onAudio.data(),_onAudio.size());
		else
			amf.writeString(_onVideo.data(),_onVideo.size());
		amf.amf0 = false;
		amf.writeNumber(time);
		amf.writeBytes(data, size);
		PacketReader reader(amf.packet.data(), amf.packet.size());
		write(writer, Writer::DATA_USER, MIME::AMF,reader);
		return;
	}
	writeFrame(writer,track&AUDIO ? AMF::AUDIO : AMF::VIDEO,time,data,size);
}

// Write data packet (custom or metadata)
void FLV::write(BinaryWriter& writer, Writer::DataType dataType, MIME::Type packetType, PacketReader& packet) {
	if (packetType == MIME::AMF) {
		writeFrame(writer,AMF::DATA, 0, packet.current(), packet.available());
		return;
	}
	// convert to AMF!
	unique_ptr<DataReader> pReader;
	if (!MIME::CreateDataReader(packetType, packet,poolBuffers, pReader)) {
		ERROR("Impossible to convert streaming ", packetType, " data to AMF, data ignored")
		return;
	}
	AMFWriter amf(poolBuffers); // TODO MONASERVER2 => write directly on BinaryWriter
	if (DataReader::STRING == pReader->nextType()) {
		// Write the handler name in AMF0!
		amf.amf0 = true;
		pReader->read(amf, 1);
		amf.amf0 = false;
	}
	pReader->read(amf); // to AMF
	writeFrame(writer,AMF::DATA, 0, amf.packet.data(), amf.packet.size());
}


void FLV::writeFrame(BinaryWriter& writer,UInt8 type,UInt32 time,const UInt8* data,UInt32 size) {
	/// 11 bytes of header
	writer.write8(type);
	// size on 3 bytes
	writer.write24(size);
	// time on 3 bytes
	writer.write24(time);
	// unknown 4 bytes set to 0
	writer.write32(0);
	/// payload
	writer.write(data, size);
	/// footer
	writer.write32(11+size);
}


////////////////////  RTP  /////////////////////////////

void RTP::write(BinaryWriter& writer,UInt8 track,UInt32 time,const UInt8* data,UInt32 size) {
	if (!data || !size)
		return;

	if(!_counter) { // First packet
		if(track&VIDEO) {
			MediaCodec::Video codec = MediaCodec::GetVideoType(*data);
			if (codec != MediaCodec::VIDEO_H264)
				WARN("Video codec for type ", codec, " is not supported yet")
		}
		else {
			MediaCodec::Audio codec = MediaCodec::GetAudioType(*data);
			if (codec != MediaCodec::AUDIO_MP3)
				INFO("Audio Codec for type : ", codec, " is not supported yet")
		}
	}
	_counter++;

	/// RTP Header
	writer.write8(0x80);		// Version (2), padding and extension (0)

	/// Payload
	if (track&VIDEO) {

		// h264 NAL Parsing
		SubstreamMap subReader(data, size);
		MPEGTS::ParseNAL(subReader, data, size);
		bool aggregated = subReader.count() > 1;

		writer.write8(0x61 + ((aggregated)? 0x80 : 00));		// Marker and Payload type (97)
		writer.write16(_counter);	// Sequence number
		writer.write32(time*90);	// Timestamp
		writer.write(_SSRC);		// SSRC
		if(aggregated) {
			writer.write8(0x18 + ((*data==0x17)? 0x60 : 0x00)); // RTP header (F|NRI|Type)
			_octetCount++;
		}

		UInt8* pos = NULL;
		UInt32 readed = subReader.readNextSub(pos, size);
		while (readed) {
			if (aggregated)
				writer.write16(readed); // Size of NALU
			
			// Write NALU
			writer.write(pos, readed);
			_octetCount += readed + 2*aggregated;

			// Read raw data unit (Video : NALU)
			readed = subReader.readNextSub(pos, size);
		}
	} else if(size>1) { // Audio (mpeg4)

		writer.write8(0x8e);	// Marker (1) and Payload type (14)
		//writer.write8(0x62);	// Marker (0) and Payload type (98)
		writer.write16(_counter);	// Sequence number
		writer.write32(time*22);	// Timestamp
		writer.write(_SSRC);		// SSRC

		writer.write32(0);
		writer.write((data+1), size-1);
		_octetCount += size+3;
	}
}

void RTP::writeRTCP(BinaryWriter& writer,UInt8 type,UInt32 time) {

	/// RTCP Header
	writer.write(EXPAND("\x80\xc8\x00\x06"));		// Version (2), padding and Reception report count (0), packet type = 200, lenght = 6
	writer.write(_SSRC);							// SSRC
	writer.write64(0);								// NTP Timestamp (not needed)
	writer.write32(time);							// RTP Timestamp
	writer.write32(_counter);						// Packet count
	writer.write32(_octetCount);					// Octet count
}


////////////////////  MPEG_TS  /////////////////////////////	

// TODO Add CRC32 and refactorize MPEGTS header
/*UInt32 MediaContainer::MPEGTS::CrcTab[256] = {
  0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
  0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
  0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
  0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
  0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
  0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
  0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
  0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
  0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
  0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
  0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
  0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
  0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
  0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
  0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
  0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
  0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
  0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
  0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
  0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
  0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
  0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
  0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
  0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
  0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
  0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
  0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
  0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
  0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
  0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
  0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
  0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
  0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
  0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
  0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
  0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
  0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
  0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
  0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
  0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
  0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
  0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
  0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

UInt32 MediaContainer::MPEGTS::CalcCrc32(UInt8 * data, UInt32 datalen) {
  UInt32 i;
  UInt32 crc = 0xffffffff;

  for (i = 0; i < datalen; i++) {
    crc = (crc << 8) ^ CrcTab[((crc >> 24) ^ *data++) & 0xff];
  }

  return crc;
}*/

#define MPEGTS_PACKET_SIZE		188
#define TS_PROGRAM_ID_AUDIO		0x60
#define TS_PROGRAM_ID_VIDEO		0x40

// Write header
void MPEGTS::write(BinaryWriter& writer,UInt8 track) {
	
	// TODO Deal with audio stream only + refactorize
	writer.write(EXPAND("\x47\x40\x00\x30\xa6\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x00\xb0\x0d\x00\x01\xc1\x00\x00\x00\x01\xe0\x20\xa2\xc3\x29\x41"))
		.write(EXPAND("\x47\x40\x20\x30\x9c\x00\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x00\x02\xb0\x17\x00\x01\xc1\x00\x00\xe0\x40\xf0\x00\x1b\xe0\x40\xf0\x00\x0f\xe0\x60\xf0\x00\x66\x75\xc4\x68"));
}

UInt32 MPEGTS::ParseNAL(SubstreamMap& reader, const UInt8* data, UInt32 size) {

	// Frame size error
	UInt32 available = 0;
	UInt8 offset = (*data==0x17 && *(data+1)==0x00)? 11 : 5; // (to work with FMLE)
	if (size<offset)
		return available;
	
	const UInt8* cur(data + offset);
	const UInt8* end(data + size);
	UInt8 twoBytesHeader = (*data==0x17 && *(data+1)==0x00) ? 1 : 0;

	// Parse each NALU
	while((end-cur) > (twoBytesHeader ? 2 : 4)) {

		if (*cur++ != 0x00) {
			WARN("H264 NALU Stream first byte not expected : ", *(cur-1))
			Logs::Dump("MPEGTS NALU with wrong first byte", data, size);
			break;
		}

		// Get size to read
		UInt32 toRead = *cur++;
		if (!twoBytesHeader) {
			toRead <<= 16;
			toRead += *cur++ << 8;
			toRead += *cur++;
		}

		if (toRead > (end-cur)) {
			WARN("H264 NALU Stream size too long : ", toRead)
			Logs::Dump("MPEGTS too long NALU", data, size);
			break;
		}

		reader.addSub(cur-data, toRead);
		cur += toRead-1;

		// Go to next NALU (two bytes header? need to ignore 0x01 byte)
		if ((end-cur) > (1 + twoBytesHeader))
			cur += 1 + twoBytesHeader;
	}

	// No NALU founded
	available = reader.totalSize();
	if (available == 0)
		return available;
	
	return available + reader.count()*3; // Add 3 bytes for each NALU start codes
}

UInt32 MPEGTS::ParseAudio(SubstreamMap& reader, const UInt8* data, UInt32 size) {

	// Frame size error
	if (size <= 2)
		return 0;

	reader.addSub(2, size-2);
	return size-2;
}

// Writer audio or video packet
void MPEGTS::write(BinaryWriter& writer,UInt8 track,UInt32 time,const UInt8* data,UInt32 size) {
	
	// TODO Parse the frame in the listener for improving performance
	SubstreamMap subReader((UInt8 *)data, size);
	if (track&VIDEO) {

		UInt32 available = ParseNAL(subReader, (UInt8 *)data, size);
		writeTS(writer, available, time, subReader, *data == 0x17, VIDEO, true);
	} else if (track&AUDIO) {

		UInt32 available = ParseAudio(subReader, (UInt8 *)data, size);
		writeTS(writer, available, time, subReader, true, AUDIO, true);
	}
}

void MPEGTS::writeTS(BinaryWriter& writer, UInt32& available, UInt32 time, SubstreamMap& subReader, bool isMetadata, Track type, bool first) {

	// Error format
	if (available == 0) {
		WARN((type==VIDEO? "VIDEO" : "AUDIO"), " Frame is not well formated");
		Logs::Dump("MPEGTS wrong frame", subReader.originalData(), subReader.originalSize());
		return;
	}

	// sync byte
	writer.write8(0x47);

	// PES start indicator (+ not used PID part)
	writer.write8(first? 0x40 : 0x00);
	
	// PID (Packet Identifier)
	writer.write8(type == AUDIO? 0x60 : 0x40);

	// Payload (middle row)
	UInt8 toWrite = MPEGTS_PACKET_SIZE - 4;
	if (!first && available > MPEGTS_PACKET_SIZE - 4)
		writer.write8(0x10 + (_counterRow[type]++ & 0x0F));  // adaptive field off + Id
	// First and last rows
	else {
		
		// Adaptive Field
		bool adaptiveField = false;
		bool timeCode = true; // TODO see if it could be usefull to send the PCR time less often
		UInt8 adaptiveSize = GetAdaptiveSize(timeCode, available, first, adaptiveField, type);
		UInt8 nbFFBytes = 0;
		toWrite = MPEGTS_PACKET_SIZE - 4 - adaptiveSize - adaptiveField - (first * (type == AUDIO? 21 : 20));
		UInt64 pts = time*90; // PTS is 90KHz time

		// adaptive field marker + Counter
		writer.write8((adaptiveField?0x30:0x10) + (_counterRow[type]++ & 0x0F));
		
		if (adaptiveField) {

			// size of Adaptive Field
			writer.write8(adaptiveSize);

			if (adaptiveSize) {

				nbFFBytes = adaptiveSize - 1;

				// 6 bytes Time PCR
				if (first && timeCode) {

					nbFFBytes -= 6;
					// Adaptive timecode flag on
					writer.write8(isMetadata? 0x50 : 0x10);

					UInt64 pcr = pts << 9;
					writer.write8(( pcr >> 34 ) & 0xff);
					writer.write8(( pcr >> 26 ) & 0xff);
					writer.write8(( pcr >> 18 ) & 0xff);
					writer.write8(( pcr >> 10 ) & 0xff);
					writer.write8(0x7e | (((pcr & (1<<9)) >> 2) & 0xFF) | (((pcr & (1<<8)) >> 8 ) & 0xFF));
					writer.write8(pcr & 0xff);
				}
				else
					writer.write8(0x00); // Adaptive timecode flag off
			}
		}

		// empty cells
		for (int i = 0; i < nbFFBytes; i++)
			writer.write8(0xFF); // TODO is it more efficient to copy big 0xFFFFFF... stream?

		// PES header (21/20 bytes)
		if (first) {

			// Packet start code prefix
			writer.write(EXPAND("\x00\x00\x01"));

			// stream id (AAC : 0xc0; AC3 : 0xfd; H264 : e0)
			writer.write8(type == AUDIO? 0xc0 : 0xe0); 	
			
			// PES size (0 means no limit)
			writer.write16(0);

			// Marker bit + Original/Copy flag + PTS flag and 
			writer.write16(0x8180);

			// PES header length that remain (5 for the PTS time)
			writer.write8(0x05);

			// Calcul of PTS time 
			writer.write8(((pts >> 29) & 0x06) + 0x21);
			writer.write8((pts >> 22) & 0xFF);
			writer.write8(((pts >> 14) & 0xFE) + 0x01);
			writer.write8((pts >> 7) & 0xFF);
			writer.write8(((pts << 1) & 0xFE) + 0x01);

			// End of header
			if (type == VIDEO) {
				writer.write32(1); // 0x00 0x00 0x00 0x01
				writer.write8(0x09);
				writer.write8(isMetadata? 0x10 : 0x30);
			} else {
				writer.write32(0xfff15040);
				writer.write16(((toWrite+7) << 5) + 0x1F);
				writer.write8(0xfc);
			}
		}
	}
	
	FATAL_ASSERT(toWrite<=available);
	available -= toWrite;

	// Write stream and continue with next row
	if (WritePES(writer, type, subReader, toWrite) && available)
		writeTS(writer, available, time, subReader, isMetadata, type, false);
}

UInt8 MPEGTS::GetAdaptiveSize(bool time, UInt32 available, bool first, bool& adaptiveField, Track type) {
	
	UInt8 size = 0;
	UInt8 sizeFree = MPEGTS_PACKET_SIZE - 4 - (first * (type == AUDIO? 21 : 20));
	adaptiveField = (available < sizeFree) || (first && time);

	// PES smaller than area free size => need to fill with 0xFF
	if (available < --sizeFree)
		size += sizeFree - available;

	// Add time if needed
	if (first && time) 
		size = max(size, (UInt8)7); // 7 : 1 byte for marker and 6 for timecode

	return size;
}

bool MPEGTS::WritePES(BinaryWriter& writer, Track type, SubstreamMap& subReader, UInt8& toWrite) {

	UInt8* pos = NULL;
	UInt32 readed = 0;
	while (toWrite > 0) {
		
		// New NALU => start code
		if (type == VIDEO && subReader.nextSubIsNew()) {
			writer.write24(1); // 0x000001
			toWrite -= 3;
		}

		// Read raw data unit (Video : NALU)
		readed = subReader.readNextSub(pos, toWrite);
		if (!readed) {
			WARN("End of substream before expected")
			Logs::Dump("MPEGTS wrong NALU", subReader.originalData(), subReader.originalSize());
			return false;
		}

		// Write
		writer.write(pos, readed);
		toWrite -= readed;
	}

	return true;
}

} // namespace Mona
