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
#include "Mona/SocketAddress.h"
#include "Mona/PoolBuffer.h"
#include "Mona/Binary.h"

namespace Mona {

class BinaryWriter : public virtual Object {
public:

	BinaryWriter(BinaryWriter& other,UInt32 offset=0);
	BinaryWriter(UInt8* buffer, UInt32 size, Binary::Order byteOrder = Binary::ORDER_BIG_ENDIAN); // ORDER_BIG_ENDIAN==NETWORK_ENDIAN
   

	template <typename ...Args>
	BinaryWriter& writeRaw(const UInt8* value, UInt32 size,Args&&... args) {
		Buffer& buffer = this->buffer();
		UInt32 oldSize(buffer.size());
		if(buffer.resize(size+oldSize, true))
			memcpy(buffer.data()+oldSize,value,size);
		else
			memcpy(buffer.data()+oldSize,value,buffer.size()-oldSize);
		return writeRaw(args ...);
	}
	template <typename ...Args>
	BinaryWriter& writeRaw(const char* value, Args&&... args) {
		return writeRaw((const UInt8*)value, strlen(value), args ...);
	}
	template <typename ...Args>
	BinaryWriter& writeRaw(const std::string& value,Args&&... args) {
		return writeRaw((const UInt8*)value.c_str(),value.size(),args ...);
	}

	BinaryWriter& write8(UInt8 value) { return writeRaw((const UInt8*)&value, sizeof(value)); }
	BinaryWriter& write16(UInt16 value);
	BinaryWriter& write24(UInt32 value);
	BinaryWriter& write32(UInt32 value);
	BinaryWriter& write64(UInt64 value);
	BinaryWriter& writeString8(const std::string& value) { write8(value.size()); return writeRaw(value); }
	BinaryWriter& writeString8(const char* value) { UInt32 size(strlen(value));  write8(size); return writeRaw((const UInt8*)value, size); }
	BinaryWriter& writeString16(const std::string& value) { write16(value.size()); return writeRaw(value); }
	BinaryWriter& writeString16(const char* value) { UInt32 size(strlen(value)); write16(size); return writeRaw((const UInt8*)value, size); }
	BinaryWriter& writeString(const std::string& value) { write7BitEncoded(value.size()); return writeRaw(value); }
	BinaryWriter& writeString(const char* value) { UInt32 size(strlen(value)); write7BitEncoded(size); return writeRaw((const UInt8*)value, size); }
	BinaryWriter& write7BitEncoded(UInt32 value);
	BinaryWriter& write7BitValue(UInt32 value);
	BinaryWriter& write7BitLongValue(UInt64 value);
	BinaryWriter& writeBool(bool value) { return write8(value ? 1 : 0); }
	template<typename NumberType>
	BinaryWriter& writeNumber(NumberType value) { return writeRaw(_flipBytes ? Binary::ReverseBytes((UInt8*)&value, sizeof(value)) : (const UInt8*)&value , sizeof(value)); }

	BinaryWriter& writeAddress(const SocketAddress& address,bool publicFlag);
	BinaryWriter& writeRandom(UInt32 count=1);
	
	virtual BinaryWriter&	clear(UInt32 size = 0) { buffer().resize(size, true); return *this; }
	BinaryWriter&	next(UInt32 count = 1);
	BinaryWriter&	clip(UInt32 offset) {buffer().clip(offset); return *this;}

	const UInt8*	data() { return buffer().data(); }
	UInt32			size() { return buffer().size(); }

private:
	
	BinaryWriter& writeRaw() {return *this;}

	virtual Buffer&	buffer() { return _buffer; }

	bool	_flipBytes;
	Buffer	_buffer;
};



} // namespace Mona
