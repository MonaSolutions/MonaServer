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

	enum VideoCodec {
		CODEC_RGB = 0,
		CODEC_RLE,
		CODEC_SORENSON,
		CODEC_SCREEN1,
		CODEC_VP6,
		CODEC_VP6_ALPHA,
		CODEC_SCREEN2,
		CODEC_H264,
		CODEC_H263,
		CODEC_MPEG4_2,
		CODEC_VIDEO_UNKNOWN = 15
	};

	enum AudioCodec {
		CODEC_PCM = 0,
		CODEC_ADPCM,
		CODEC_MP3,
		CODEC_PCM_LITTLE,
		CODEC_NELLYMOSER_16,
		CODEC_NELLYMOSER_8,
		CODEC_NELLYMOSER_ANY,
		CODEC_ALAW,
		CODEC_ULAW,
		CODEC_AAC = 10,
		CODEC_SPEEX,
		CODEC_MP3_8 = 14,
		CODEC_AUDIO_UNKNOWN
	};

	static VideoCodec GetVideoType(UInt8 marker) { if ((marker&0x0F)>9) return CODEC_VIDEO_UNKNOWN; else return (VideoCodec)(marker&0x0F); }
	static AudioCodec GetAudioType(UInt8 marker) { 
		switch (marker>>4)  {
		case 9:
		case 12:
		case 13:
		case 15:
			return CODEC_AUDIO_UNKNOWN;
		default:
			return (AudioCodec)(marker>>4); 
		}
	}

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
