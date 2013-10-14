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
#include "Mona/BinaryWriter.h"

namespace Mona {

class MemoryWriter: public BinaryWriter {
public:
	MemoryWriter(const Mona::UInt8* buffer,Mona::UInt32 size);
	MemoryWriter(MemoryWriter&);
	virtual ~MemoryWriter();

	Mona::UInt8*		begin();
	Mona::UInt32		length();
	Mona::UInt32		position();
	
	Mona::UInt32		available();

	bool	good();
	void	clear(Mona::UInt32 pos=0);
	void	reset(Mona::UInt32 newPos);
	void	limit(Mona::UInt32 length=0);
	void	next(int size=1);
	void	flush();
	void	clip(Mona::UInt32 offset);

private:
	MemoryOutputStream	_memory;
	MemoryWriter*		_pOther;
	Mona::UInt32		_size;
};

inline void MemoryWriter::clip(Mona::UInt32 offset) {
	_memory.clip(offset);
}
inline Mona::UInt32 MemoryWriter::available() {
	return _memory.available();
}
inline bool MemoryWriter::good() {
	return _memory.good();
}
inline Mona::UInt32 MemoryWriter::length() {
	return _memory.written();
}
inline Mona::UInt32 MemoryWriter::position() {
	return _memory.current()-(char*)begin();
}
inline void MemoryWriter::reset(Mona::UInt32 newPos) {
	_memory.reset(newPos);
}
inline void MemoryWriter::next(int size) {
	return _memory.next(size);
}

inline Mona::UInt8* MemoryWriter::begin() {
	return (Mona::UInt8*)_memory.begin();
}


} // namespace Mona
