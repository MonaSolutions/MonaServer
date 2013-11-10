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
	virtual ~MemoryWriter();

	UInt8*		begin() { return (UInt8*)_memory.begin(); }
	UInt32		length() { return _memory.written(); }
	UInt32		position() { return _memory.current() - (char*)begin(); }
	
	UInt32		available() { return _memory.available(); }

	bool	good() { return _memory.good(); }
	void	reset(UInt32 newPos) { _memory.reset(newPos); }
	void	next(int size = 1) { return _memory.next(size); }
	void	clip(UInt32 offset) { _memory.clip(offset); }

	void	limit(UInt32 length = 0);
	void	clear(UInt32 pos = 0);
	void	flush();

private:
	MemoryOutputStream	_memory;
	MemoryWriter*		_pOther;
	UInt32		_size;
};


} // namespace Mona
