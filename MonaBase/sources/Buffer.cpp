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



namespace Mona {

Buffer	Buffer::Null(true,true,true);

void Buffer::clip(UInt32 offset) {
	if (offset > _size)
		offset = _size;
	_offset += offset;
	if (offset == 0)
		return;
	_size -= offset;
	_data += offset;
	_capacity -= offset;
}

void Buffer::clear() {
	_size = 0;
	// fix possible clip
	if (_offset>0) {
		_capacity += _offset;
		_data -= _offset;
		_offset = 0;
	}
}

bool Buffer::resize(UInt32 size,bool preserveData) {
	if (size <= _capacity) {
		_size = size;
		return true;
	}

	if (!_buffer) {
		_size = _capacity;
		return false;
	}
	
	do {
		_capacity *= 2;
	} while (size > _capacity);

	UInt8* oldData = _data;
	_data = new UInt8[_capacity]();
	if (preserveData && size>0)
		memcpy(_data, oldData, _size);
	delete [] _buffer;
	_buffer=_data;
	_size = size;
	return true;
}



} // namespace Mona
