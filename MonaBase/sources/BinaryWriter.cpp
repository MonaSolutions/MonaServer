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

#include "Mona/BinaryWriter.h"
#include "Mona/Binary.h"
#include "Mona/Util.h"

using namespace std;


namespace Mona {


BinaryWriter::BinaryWriter(UInt8* buffer, UInt32 size,Binary::Order byteOrder) : _buffer(buffer,size) {
	_buffer.resize(0);
#if defined(_ARCH_BIG_ENDIAN)
	_flipBytes = byteOrder == Binary::ORDER_LITTLE_ENDIAN;
#else
    _flipBytes = byteOrder == Binary::ORDER_BIG_ENDIAN;;
#endif
}


BinaryWriter& BinaryWriter::write(const void* value, UInt32 size) {
	Buffer& buffer = this->buffer();
	UInt32 oldSize(buffer.size());
	if(buffer.resize(size+oldSize, true))
		memcpy(buffer.data()+oldSize,value,size);
	else
		memcpy(buffer.data()+oldSize,value,buffer.size()-oldSize);
	return *this;
}

BinaryWriter& BinaryWriter::writeRandom(UInt32 count) {
	while(count--)
		write8(Util::Random<UInt8>());
	return *this;
}

BinaryWriter& BinaryWriter::next(UInt32 count) {
	Buffer& buffer(this->buffer());
	buffer.resize(buffer.size() + count,true);
	return *this;
}

BinaryWriter& BinaryWriter::write7BitEncoded(UInt32 value) {
	do {
		unsigned char c = (unsigned char)(value & 0x7F);
		value >>= 7;
		if (value)
			c |= 0x80;
		write8(c);
	} while (value);
	return *this;
}

BinaryWriter& BinaryWriter::write16(UInt16 value) {
	if (_flipBytes)
		value = Binary::Flip16(value);
	return write(&value, sizeof(value));
}

BinaryWriter& BinaryWriter::write24(UInt32 value) {
	if (_flipBytes)
		value = Binary::Flip24(value);
	return write(&value, 3);
}

BinaryWriter& BinaryWriter::write32(UInt32 value) {
	if (_flipBytes)
		value = Binary::Flip32(value);
	return write(&value, sizeof(value));
}

BinaryWriter& BinaryWriter::write64(UInt64 value) {
	if (_flipBytes)
		value = Binary::Flip64(value);
	return write(&value, sizeof(value));
}

BinaryWriter& BinaryWriter::write7BitValue(UInt32 value) {
	UInt8 shift = (Util::Get7BitValueSize(value)-1)*7;
	bool max = false;
	if(shift>=21) { // 4 bytes maximum
		shift = 22;
		max = true;
	}

	while(shift>=7) {
		write8(0x80 | ((value>>shift)&0x7F));
		shift -= 7;
	}
	return write8(max ? value&0xFF : value&0x7F);
}

BinaryWriter& BinaryWriter::write7BitLongValue(UInt64 value) {
	UInt8 shift = (Util::Get7BitValueSize(value)-1)*7;
	bool max = shift>=63; // Can give 10 bytes!
	if(max)
		++shift;

	while(shift>=7) {
		write8(0x80 | ((value>>shift)&0x7F));
		shift -= 7;
	}
	return write8(max ? value&0xFF : value&0x7F);
}


} // namespace Mona
