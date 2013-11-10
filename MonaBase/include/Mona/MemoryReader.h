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
#include "Mona/MemoryStream.h"
#include "Mona/BinaryReader.h"


namespace Mona {


class MemoryReader: public BinaryReader, virtual Object {
public:
	MemoryReader(const UInt8* buffer,UInt32 size);
	
	const UInt32	fragments;

	UInt32	available() { return _memory.available(); }
	UInt8*	current() { return (UInt8*)_memory.current(); }
	UInt32	position() { return _memory.current() - _memory.begin(); }

	void			reset(UInt32 newPos = 0) { _memory.reset(newPos); }
	void			shrink(UInt32 rest);
	void			next(UInt32 count = 1) { return _memory.next(count); }

	static MemoryReader Null;
private:
	MemoryInputStream _memory;
	
};


} // namespace Mona
