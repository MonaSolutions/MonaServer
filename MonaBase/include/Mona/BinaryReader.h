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


class BinaryReader : public Binary, virtual public Object {
public:
   
    BinaryReader(const UInt8* data,UInt32 size,Binary::Order byteOrder=Binary::ORDER_NETWORK);

	
	UInt8*			read(UInt32 size, UInt8* buffer);
	char*			read(UInt32 size, char*  buffer) { return (char*)read(size,(UInt8*)buffer); }
	char			read() { return _current==_end ? 0 : *_current++; }
	template<typename BufferType>
	BufferType&		read(UInt32 size, BufferType& buffer) {
		buffer.resize(size);
		read(size,(UInt8*)buffer.data());
		return buffer;
	}

	UInt32			read7BitValue();
	UInt64			read7BitLongValue();
	UInt32			read7BitEncoded();
	std::string&	readString(std::string& value) { return read(read7BitEncoded(),value); }
	UInt8			read8() { return _current==_end ? 0 : *_current++; }
	UInt16			read16();
	UInt32			read24();
	UInt32			read32();
	UInt64			read64();
	bool			readBool() { return _current==_end ? false : ((*_current++) != 0); }
	template<typename NumberType>
	NumberType		readNumber() {
		NumberType value;
		read(sizeof(value),(UInt8*)&value);
		if (_flipBytes)
			Binary::ReverseBytes(&value, sizeof(value));
		return value;
	}

	UInt32			position() const { return _current-_data; }
	void			next(UInt32 count = 1) { _current += (count > available() ? available() : count); }
	void			reset(UInt32 position = 0) { _current = _data+(position > _size ? _size : position); }
	void			shrink(UInt32 rest);

	const UInt8*	current() const { return _current; }
	UInt32			available() const { return _end-_current; }

	// beware, data() can be null
	const UInt8*	data() const { return _data; }
	UInt32			size() const { return _size; }

private:
	
	bool			_flipBytes;
	const UInt8*	_data;
	const UInt8*	_end;
	const UInt8*	_current;
	UInt32			_size;
};


} // namespace Mona
