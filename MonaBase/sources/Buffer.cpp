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

#include "Mona/Buffer.h"
#include "Mona/PacketReader.h"
#include "Mona/PacketWriter.h"



namespace Mona {

PacketReader PacketReader::Null(NULL,0);
PacketWriter PacketWriter::Null(Buffer::Null);
Buffer	Buffer::Null(NULL,0);

void Buffer::clip(UInt32 offset) {
	if (offset > _size)
		offset = _size;
	if (offset == 0)
		return;
	_offset += offset;
	_data += offset;
	_size -= offset;
	_capacity -= offset;
}


bool Buffer::resize(UInt32 size,bool preserveData) {
	if (size <= _capacity) {
		if (_offset && !preserveData) {
			// fix possible clip
			_capacity += _offset;
			_data -= _offset;
			_offset = 0;
		}
		_size = size;
		return true;
	}

	// here size > capacity, so size > _size

	UInt8* oldData(_data);

	// try without offset
	if (_offset) {
		_capacity += _offset;
		_data -= _offset;
		_offset = 0;
		if (size <= _capacity) {
			if (preserveData)
				memmove(_data,oldData,_size);
			_size = size;
			return true;
		}
		if (!_buffer && preserveData)
			memmove(_data, oldData, _size);
	}

	if (!_buffer) {
		_size = _capacity;
		return false;
	}

	if (_capacity == 0)
		_capacity = 32;
	
	do {
		_capacity *= 2;
	} while (_capacity && size > _capacity);
	if (_capacity == 0) // to expect the case or _capacity*2 = 0 (exceeds maximum size)
		_capacity = size;

	_data = new UInt8[_capacity]();
	if (preserveData)
		memcpy(_data, oldData, _size);
	delete [] _buffer;
	_size = size;
	_buffer=_data;
	return true;
}



} // namespace Mona
