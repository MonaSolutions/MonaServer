/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
#include "Mona/Util.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {

BinaryWriter BinaryWriter::Null(Util::NullOutputStream);

BinaryWriter::BinaryWriter(ostream& ostr) : Poco::BinaryWriter(ostr,BinaryWriter::NETWORK_BYTE_ORDER) {
}


BinaryWriter::~BinaryWriter() {
	flush();
}

void BinaryWriter::write24(Mona::UInt32 value) {
	write8(value>>16);
	write16(value);
}


void BinaryWriter::writeString8(const char* value,UInt8 size) {
	write8(size);
	writeRaw(value,size);
}
void BinaryWriter::writeString8(const string& value) {
	write8(value.size());
	writeRaw(value);
}
void BinaryWriter::writeString16(const char* value,UInt16 size) {
	write16(size);
	writeRaw(value,size);
}
void BinaryWriter::writeString16(const string& value) {
	write16(value.size());
	writeRaw(value);
}

void BinaryWriter::writeAddress(const SocketAddress& address,bool publicFlag) {
	UInt8 flag = publicFlag ? 0x02 : 0x01;
	UInt8 size = 4;
	IPAddress host = address.host();
	if(host.family() == IPAddress::IPv6) {
		flag |= 0x80;
		size = 16;
	}
	const UInt8* bytes = reinterpret_cast<const UInt8*>(host.addr());
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
