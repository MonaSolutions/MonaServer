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

#include "Test.h"
#include "Mona/BinaryWriter.h"
#include "Mona/BinaryReader.h"

using namespace Mona;
using namespace std;

static Buffer _Buffer(128);

void Write(BinaryWriter& writer) {

	bool bval = true;
	writer.write(&bval, sizeof(bval));
	bval = false;
	writer.write(&bval, sizeof(bval));
	writer.write8('a');
	writer.write16(-100);
	writer.write16(50000);
	writer.write32(-123456);
	writer.write32(123456);
	writer.write32(-1234567890);
	writer.write32(1234567890);

	writer.write64(-1234567890);
	writer.write64(1234567890);

	float fVal = 1.5;
	writer.write(&fVal, sizeof(fVal));
	double dVal = -1.5;
	writer.write(&dVal, sizeof(dVal));

	writer.write7BitValue(100);
	writer.write7BitValue(1000);
	writer.write7BitValue(10000);
	writer.write7BitValue(100000);
	writer.write7BitValue(1000000);

	writer.write7BitLongValue(100);
	writer.write7BitLongValue(1000);
	writer.write7BitLongValue(10000);
	writer.write7BitLongValue(100000);
	writer.write7BitLongValue(1000000);

	writer.write("RAW");
}


void Read(BinaryReader& reader) {
	bool b;
	reader.read(sizeof(b), (char*)&b);
	CHECK(b);
	reader.read(sizeof(b), (char*)&b);
	CHECK(!b);

	char c = reader.read8();
	CHECK(c == 'a');

	short shortv = reader.read16();
	CHECK(shortv == -100);

	unsigned short ushortv = reader.read16();
	CHECK(ushortv == 50000);

	int intv = reader.read32();
	CHECK(intv == -123456);

	unsigned uintv = reader.read32();
	CHECK(uintv == 123456);

	Int32 longv = reader.read32();
	CHECK(longv == -1234567890);

	UInt32 ulongv = reader.read32();
	CHECK(ulongv == 1234567890);

	Int64 int64v = reader.read64();
	CHECK(int64v == -1234567890);

	UInt64 uint64v = reader.read64();
	CHECK(uint64v == 1234567890);

	float floatv;
	reader.read(sizeof(floatv), (char *)&floatv);
	CHECK(floatv == 1.5);

	double doublev;
	reader.read(sizeof(doublev), (char *)&doublev);
	CHECK(doublev == -1.5);

	UInt32 uint32v = reader.read7BitValue();
	CHECK(uint32v == 100);
	uint32v = reader.read7BitValue();
	CHECK(uint32v == 1000);
	uint32v = reader.read7BitValue();
	CHECK(uint32v == 10000);
	uint32v = reader.read7BitValue();
	CHECK(uint32v == 100000);
	uint32v = reader.read7BitValue();
	CHECK(uint32v == 1000000);

	uint64v = reader.read7BitLongValue();
	CHECK(uint64v == 100);
	uint64v = reader.read7BitLongValue();
	CHECK(uint64v == 1000);
	uint64v = reader.read7BitLongValue();
	CHECK(uint64v == 10000);
	uint64v = reader.read7BitLongValue();
	CHECK(uint64v == 100000);
	uint64v = reader.read7BitLongValue();
	CHECK(uint64v == 1000000);

	CHECK(String::ICompare((const char*)reader.current(),"RAW")==0);
}


ADD_TEST(BinaryReaderWriterTest, Native) {
	BinaryWriter writer(_Buffer.data(),_Buffer.size());
	Write(writer);
	BinaryReader reader(writer.data(),writer.size());
	Read(reader);
}

ADD_TEST(BinaryReaderWriterTest, BigEndian) {
	BinaryWriter writer(_Buffer.data(),_Buffer.size(), Binary::ORDER_BIG_ENDIAN);
	Write(writer);

	BinaryReader reader(writer.data(),writer.size(), Binary::ORDER_BIG_ENDIAN);
	Read(reader);
}

ADD_TEST(BinaryReaderWriterTest, LittleEndian) {
    BinaryWriter writer(_Buffer.data(),_Buffer.size(), Binary::ORDER_LITTLE_ENDIAN);
	Write(writer);

	BinaryReader reader(writer.data(),writer.size(), Binary::ORDER_LITTLE_ENDIAN);
	Read(reader);
}
