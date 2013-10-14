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


class MemoryReader: public BinaryReader {
public:
	MemoryReader(const Mona::UInt8* buffer,Mona::UInt32 size);
	MemoryReader(MemoryReader&);
	virtual ~MemoryReader();

	const Mona::UInt32	fragments;

	Mona::UInt32	available();
	Mona::UInt8*	current();
	Mona::UInt32	position();

	void			reset(Mona::UInt32 newPos=0);
	void			shrink(Mona::UInt32 rest);
	void			next(Mona::UInt32 size=1);

	static MemoryReader Null;
private:
	MemoryInputStream _memory;
	
};

inline Mona::UInt32 MemoryReader::available() {
	return _memory.available();
}

inline Mona::UInt32 MemoryReader::position() {
	return _memory.current()-_memory.begin();
}

inline void MemoryReader::reset(Mona::UInt32 newPos) {
	_memory.reset(newPos);
}

inline void MemoryReader::next(Mona::UInt32 count) {
	return _memory.next(count);
}

inline Mona::UInt8* MemoryReader::current() {
	return (Mona::UInt8*)_memory.current();
}


} // namespace Mona
