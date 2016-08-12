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
#include "Mona/Binary.h"

namespace Mona {

class Buffer : public Binary, virtual public NullableObject {
public:
	Buffer() : _offset(0), _size(0),_capacity(32) {_data = _buffer = new UInt8[_capacity]();}
	Buffer(UInt32 size) : _offset(0),_size(size),_capacity(size) {_data = _buffer = new UInt8[_capacity]();}
	Buffer(UInt8* buffer,UInt32 size) : _offset(0),_data(buffer), _size(size),_capacity(size),_buffer(NULL) {}
	virtual ~Buffer() { if (_buffer) delete [] _buffer; }

	void			clip(UInt32 offset);
	void			append(const void* data, UInt32 size) { Append(*this, data, size); }
	bool			resize(UInt32 size, bool preserveContent=true);
	void			clear() { resize(0, false); }

	// beware, data() can be null
	UInt8*			data() { return _data; }
	const UInt8*	data() const { return _data; }
	UInt32			size() const { return _size; }

	UInt32			capacity() const { return _capacity; }

	operator bool() const { return _data != NULL;  }
	
	static Buffer Null;

	template <typename BufferType>
	static BufferType& Append(BufferType& buffer, const void* data, UInt32 size) {
		if (!buffer.data()) // to expect null writer 
			return buffer;
		UInt32 oldSize(buffer.size());
		buffer.resize(oldSize + size);
		memcpy((UInt8*)buffer.data() + oldSize, data, size);
		return buffer;
	}

private:
	UInt32  _offset;
	UInt8*	_data;
	UInt32	_size;
	UInt32	_capacity;
	UInt8*	_buffer;
};




} // namespace Mona
