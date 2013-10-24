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

class MemoryWriter: public BinaryWriter, virtual Object {
public:
	MemoryWriter(const UInt8* buffer,UInt32 size);
	//MemoryWriter(MemoryWriter&);
	virtual ~MemoryWriter();

	UInt8*		begin();
	UInt32		length();
	UInt32		position();
	
	UInt32		available();

	bool	good();
	void	clear(UInt32 pos=0);
	void	reset(UInt32 newPos);
	void	limit(UInt32 length=0);
	void	next(int size=1);
	void	flush();
	void	clip(UInt32 offset);

private:
	MemoryOutputStream	_memory;
	MemoryWriter*		_pOther;
	UInt32		_size;
};

inline void MemoryWriter::clip(UInt32 offset) {
	_memory.clip(offset);
}
inline UInt32 MemoryWriter::available() {
	return _memory.available();
}
inline bool MemoryWriter::good() {
	return _memory.good();
}
inline UInt32 MemoryWriter::length() {
	return _memory.written();
}
inline UInt32 MemoryWriter::position() {
	return _memory.current()-(char*)begin();
}
inline void MemoryWriter::reset(UInt32 newPos) {
	_memory.reset(newPos);
}
inline void MemoryWriter::next(int size) {
	return _memory.next(size);
}

inline UInt8* MemoryWriter::begin() {
	return (UInt8*)_memory.begin();
}


} // namespace Mona
