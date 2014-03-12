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
#include <map>

namespace Mona {

class Buffer : virtual NullableObject {
public:
	Buffer(UInt32 size = 0) : _offset(0),_capacity(size==0 ? 64 : size), _size(size) {_data = _buffer = new UInt8[_capacity]();}
	Buffer(UInt8* buffer,UInt32 size) : _offset(0),_buffer(NULL),_data(buffer),_capacity(size), _size(size) {}
	virtual ~Buffer() { if (_buffer) delete [] _buffer; }

	const UInt8		operator[](UInt32 index) const { return _data[index >= _size ? (_size-1) : index]; }
	UInt8&			operator[](UInt32 index) { return _data[index >= _size ? (_size-1) : index]; }

	void			clip(UInt32 offset);
	bool			resize(UInt32 size, bool preserveContent=false);
	void			clear();

	UInt8*			data() { return _data; }
	const UInt8*	data() const { return _data; }
	UInt32			size() const { return _size; }

	UInt32			capacity() const { return _capacity; }
	
	static Buffer Null;

private:
	Buffer(bool,bool,bool) : NullableObject(true),_buffer(NULL),_data(NULL),_capacity(0), _size(0) {}

	UInt32  _offset;
	UInt8*	_data;
	UInt32	_size;
	UInt32	_capacity;
	UInt8*	_buffer;
};




} // namespace Mona
