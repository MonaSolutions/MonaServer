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

#pragma once

#include "Mona/Mona.h"
#include "Mona/MemoryReader.h"
#include "Mona/BinaryWriter.h"


namespace Mona {

	
//FRAME_OP_CONT    = 0x00, /// Continuation frame.
#define WS_TEXT     0x01 /// Text frame.
#define WS_BINARY	0x02 /// Binary frame.
#define WS_CLOSE	0x08 /// Close connection.
#define WS_PING		0x09 /// Ping frame.
#define WS_PONG		0x0a /// Pong frame.

class WS {
public:

	static void			ComputeKey(std::string& key);
	static void		    Unmask(MemoryReader& data);
	static UInt8	WriteHeader(UInt8 type,UInt32 size,BinaryWriter& writer);
	static UInt8	HeaderSize(UInt32 size);

	
};


} // namespace Mona
