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

namespace Mona {




#define AMF_NULL			0x05
#define AMF_UNDEFINED		0x06
#define AMF_UNSUPPORTED		0x0D
#define	AMF_AVMPLUS_OBJECT	0x11

#define AMF_NUMBER			0x00
#define AMF_BOOLEAN			0x01
#define AMF_STRING			0x02
#define AMF_DATE			0x0B

#define AMF_BEGIN_OBJECT		0x03
#define AMF_BEGIN_TYPED_OBJECT	0x10
#define AMF_END_OBJECT			0x09
#define AMF_REFERENCE			0x07

#define AMF_MIXED_ARRAY	    0x08 
#define	AMF_STRICT_ARRAY	0x0A

#define	AMF_LONG_STRING		0x0C


#define AMF3_UNDEFINED		0x00
#define AMF3_NULL			0x01
#define AMF3_FALSE			0x02
#define AMF3_TRUE			0x03
#define AMF3_INTEGER		0x04
#define AMF3_NUMBER			0x05
#define AMF3_STRING			0x06
#define AMF3_DATE			0x08
#define AMF3_ARRAY			0x09
#define AMF3_OBJECT			0x0A
#define AMF3_BYTEARRAY		0x0C
#define AMF3_DICTIONARY		0x11

#define	AMF_END				0xFF

#define AMF_MAX_INTEGER		268435455


class AMF : virtual Static {
public:
	enum ContentType {
		EMPTY				=0x00,
		CHUNKSIZE			=0x01,
		ABORT				=0x02,
		ACK					=0x03,
		RAW					=0x04,
		WIN_ACKSIZE			=0x05,
		BANDWITH			=0x06,
		AUDIO				=0x08,
		VIDEO				=0x09,
		DATA_AMF3			=0x0F,
		INVOCATION_AMF3		=0x11,
		DATA				=0x12,
		INVOCATION			=0x14
	};
};

}
