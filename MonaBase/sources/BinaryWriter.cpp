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


BinaryWriter::BinaryWriter(ostream& ostr,ByteOrder byteOrder) : _ostr(ostr) {
#if defined(_ARCH_BIG_ENDIAN)
	_flipBytes = byteOrder == LITTLE_ENDIAN;
#else
    _flipBytes = byteOrder == BIG_ENDIAN_ORDER;;
#endif
}


BinaryWriter::~BinaryWriter() {
	_ostr.flush();
}

void BinaryWriter::write7BitEncoded(UInt32 value) {
	do {
		unsigned char c = (unsigned char)(value & 0x7F);
		value >>= 7;
		if (value)
			c |= 0x80;
		_ostr.put(c);
	} while (value);
}

void BinaryWriter::write16(UInt16 value) {
	if (_flipBytes)
		value = Binary::Flip16(value);
	_ostr.write((const char*)&value, sizeof(value));
}

void BinaryWriter::write24(UInt32 value) {
	if (_flipBytes)
		value = Binary::Flip24(value);
	_ostr.write((const char*)&value, sizeof(value));
}

void BinaryWriter::write32(UInt32 value) {
	if (_flipBytes)
		value = Binary::Flip32(value);
	_ostr.write((const char*)&value, sizeof(value));
}

void BinaryWriter::write64(UInt64 value) {
	if (_flipBytes)
		value = Binary::Flip64(value);
	_ostr.write((const char*)&value, sizeof(value));
}

void BinaryWriter::writeAddress(const SocketAddress& address,bool publicFlag) {
	UInt8 flag = publicFlag ? 0x02 : 0x01;
	const IPAddress& host = address.host();
	if (host.family() == IPAddress::IPv6)
		flag |= 0x80;
	NET_SOCKLEN size;
	const UInt8* bytes = reinterpret_cast<const UInt8*>(host.addr(size));
	write8(flag);
	for(int i=0;i<size;++i)
		write8(bytes[i]);
	write16(address.port());
}

void BinaryWriter::write7BitValue(UInt32 value) {
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
	write8(max ? value&0xFF : value&0x7F);
}

void BinaryWriter::write7BitLongValue(UInt64 value) {
	UInt8 shift = (Util::Get7BitValueSize(value)-1)*7;
	bool max = shift>=63; // Can give 10 bytes!
	if(max)
		++shift;

	while(shift>=7) {
		write8(0x80 | ((value>>shift)&0x7F));
		shift -= 7;
	}
	write8(max ? value&0xFF : value&0x7F);
}


} // namespace Mona
