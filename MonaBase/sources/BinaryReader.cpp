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

#include "Mona/BinaryReader.h"

using namespace std;

namespace Mona {

BinaryReader::BinaryReader(const UInt8* data,UInt32 size, Binary::Order byteOrder) : _end(data+size),_data(data),_size(size),_current(data) {
#if defined(_ARCH_BIG_ENDIAN)
	_flipBytes = byteOrder == Binary::ORDER_LITTLE_ENDIAN;
#else
    _flipBytes = byteOrder == Binary::ORDER_BIG_ENDIAN;
#endif
}

void BinaryReader::shrink(UInt32 rest) {
	UInt32 available(this->available());
	if (rest > available)
		rest = available;
	_end = _current+rest;
	_size = _end-_data;
}

UInt8* BinaryReader::read(UInt32 size, UInt8* value) {
	UInt32 available(this->available());
	if (size > available)
		size = available;
	if (size == 0)
		return value;
	memcpy(value, _current,size);
	_current += size;
	return value;
}

UInt32 BinaryReader::read7BitEncoded() {
	UInt8 c;
	UInt32 value = 0;
	int s = 0;
	do {
		c = read8();
		UInt32 x = (c & 0x7F);
		x <<= s;
		value += x;
		s += 7;
	} while (c & 0x80);
	return value;
}

UInt16 BinaryReader::read16() {
	UInt16 value(0);
	read(sizeof(value),(UInt8*)&value);
	if (_flipBytes)
		return Binary::Flip16(value);
	return value;
}

UInt32 BinaryReader::read24() {
	UInt32 value(0);
	read(3, (UInt8*)&value);
	if (_flipBytes)
		return Binary::Flip24(value);
	return value;
}

UInt32 BinaryReader::read32() {
	UInt32 value(0);
	read(sizeof(value), (UInt8*)&value);
	if (_flipBytes)
		return Binary::Flip32(value);
	return value;
}


UInt64 BinaryReader::read64() {
	UInt64 value(0);
	read(sizeof(value), (UInt8*)&value);
	if (_flipBytes)
		return Binary::Flip64(value);
	return value;
}

UInt32 BinaryReader::read7BitValue() {
	UInt8 n = 0;
    UInt8 b = read8();
    UInt32 result = 0;
    while ((b&0x80) && n < 3) {
        result <<= 7;
        result |= (b&0x7F);
        b = read8();
        ++n;
    }
    result <<= ((n<3) ? 7 : 8); // Use all 8 bits from the 4th byte
    result |= b;
	return result;
}

UInt64 BinaryReader::read7BitLongValue() {
	UInt8 n = 0;
    UInt8 b = read8();
    UInt64 result = 0;
    while ((b&0x80) && n < 8) {
        result <<= 7;
        result |= (b&0x7F);
        b = read8();
        ++n;
    }
    result <<= ((n<8) ? 7 : 8); // Use all 8 bits from the 4th byte
    result |= b;
	return result;
}


} // namespace Mona
