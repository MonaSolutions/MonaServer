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
#include "Mona/PoolBuffer.h"
#include "Mona/Binary.h"

namespace Mona {

class BinaryWriter : public Binary, virtual public Object {
public:

	BinaryWriter(UInt8* buffer, UInt32 size, Binary::Order byteOrder = Binary::ORDER_NETWORK);

	BinaryWriter& write(const void* value, UInt32 size);
	BinaryWriter& write(const char* value) { return write(value, strlen(value)); }
	BinaryWriter& write(char* value)	   { return write(value, strlen(value)); } // required cause the following template signature which catch "char *" on posix
	template<typename BinaryType>
	BinaryWriter& write(const BinaryType& value) { return write(value.data(),(UInt32)value.size()); }
	BinaryWriter& write(char value) { return write(&value, sizeof(value)); }

	BinaryWriter& write8(UInt8 value) { return write(&value, sizeof(value)); }
	BinaryWriter& write16(UInt16 value);
	BinaryWriter& write24(UInt32 value);
	BinaryWriter& write32(UInt32 value);
	BinaryWriter& write64(UInt64 value);
	BinaryWriter& writeString(const std::string& value) { write7BitEncoded(value.size()); return write(value); }
	BinaryWriter& writeString(const char* value) { UInt32 size(strlen(value)); write7BitEncoded(size); return write(value, size); }
	BinaryWriter& write7BitEncoded(UInt32 value);
	BinaryWriter& write7BitValue(UInt32 value);
	BinaryWriter& write7BitLongValue(UInt64 value);
	BinaryWriter& writeBool(bool value) { return write8(value ? 1 : 0); }
	template<typename NumberType>
	BinaryWriter& writeNumber(NumberType value) { return write(_flipBytes ? Binary::ReverseBytes(&value, sizeof(value)) : &value , sizeof(value)); }

	BinaryWriter& writeRandom(UInt32 count=1);
	
	virtual BinaryWriter&	clear(UInt32 size = 0) { buffer().resize(size, true); return *this; }
	BinaryWriter&			next(UInt32 count = 1);
	BinaryWriter&			clip(UInt32 offset) {buffer().clip(offset); return *this;}

	// beware, data() can be null
	UInt8*			data() { return buffer().data(); }
	const UInt8*	data() const { return const_cast<BinaryWriter*>(this)->buffer().data(); }
	UInt32			size() const { return const_cast<BinaryWriter*>(this)->buffer().size(); }

private:

	virtual Buffer&	buffer() { return _buffer; }

	bool	_flipBytes;
	Buffer	_buffer;
};



} // namespace Mona
