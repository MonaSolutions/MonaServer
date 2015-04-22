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

	enum Video {
		VIDEO_RGB = 0,
		VIDEO_RLE,
		VIDEO_SORENSON,
		VIDEO_SCREEN1,
		VIDEO_VP6,
		VIDEO_VP6_ALPHA,
		VIDEO_SCREEN2,
		VIDEO_H264,
		VIDEO_H263,
		VIDEO_MPEG4_2,
		VIDEO_UNKNOWN = 15
	};

	enum Audio {
		AUDIO_PCM = 0,
		AUDIO_ADPCM,
		AUDIO_MP3,
		AUDIO_PCM_LITTLE,
		AUDIO_NELLYMOSER_16,
		AUDIO_NELLYMOSER_8,
		AUDIO_NELLYMOSER_ANY,
		AUDIO_ALAW,
		AUDIO_ULAW,
		AUDIO_AAC = 10,
		AUDIO_SPEEX,
		AUDIO_MP3_8 = 14,
		AUDIO_UNKNOWN
	};

	static Video GetVideoType(UInt8 marker) { if ((marker&0x0F)>9) return VIDEO_UNKNOWN; else return (Video)(marker&0x0F); }
	static Audio GetAudioType(UInt8 marker) { 
		switch (marker>>4)  {
		case 9:
		case 12:
		case 13:
		case 15:
			return AUDIO_UNKNOWN;
		default:
			return (Audio)(marker>>4); 
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
