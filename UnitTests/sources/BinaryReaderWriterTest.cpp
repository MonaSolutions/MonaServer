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
#include <sstream>

using namespace Mona;
using namespace std;



void Write(BinaryWriter& writer) {

	bool bval = true;
	writer.writeBool(bval);
	bval = false;
	writer.writeBool(bval);
	writer.write8('a');
	writer.write16((short)-100);
	writer.write16((unsigned short)50000);
	writer.write32(-123456);
	writer.write32((unsigned)123456);
	writer.write32((long)-1234567890);
	writer.write32((unsigned long)1234567890);

	writer.write64((Int64)-1234567890);
	writer.write64((UInt64)1234567890);

	float fVal = 1.5;
	writer.writeRaw((char *)&fVal, sizeof(fVal));
	double dVal = -1.5;
	writer.writeRaw((char *)&dVal, sizeof(dVal));

	writer.writeString8("foo");
	writer.writeString8("");

	writer.writeString8(std::string("bar"));
	writer.writeString8(std::string());

	writer.write7BitValue((UInt32)100);
	writer.write7BitValue((UInt32)1000);
	writer.write7BitValue((UInt32)10000);
	writer.write7BitValue((UInt32)100000);
	writer.write7BitValue((UInt32)1000000);

	writer.write7BitLongValue((UInt64)100);
	writer.write7BitLongValue((UInt64)1000);
	writer.write7BitLongValue((UInt64)10000);
	writer.write7BitLongValue((UInt64)100000);
	writer.write7BitLongValue((UInt64)1000000);

	writer.writeRaw("RAW");
}


void Read(BinaryReader& reader) {

	bool b = reader.readBool();
	CHECK(b);
	b = reader.readBool();
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

	long longv = reader.read32();
	CHECK(longv == -1234567890);

	unsigned long ulongv = reader.read32();
	CHECK(ulongv == 1234567890);

	Int64 int64v = reader.read64();
	CHECK(int64v == -1234567890);

	UInt64 uint64v = reader.read64();
	CHECK(uint64v == 1234567890);

	float floatv;
	reader.readRaw((char *)&floatv, sizeof(floatv));
	CHECK(floatv == 1.5);

	double doublev;
	reader.readRaw((char *)&doublev, sizeof(doublev));
	CHECK(doublev == -1.5);

	std::string str;
	reader.readString8(str);
	CHECK(str == "foo");
	reader.readString8(str);
	CHECK(str == "");
	reader.readString8(str);
	CHECK(str == "bar");
	reader.readString8(str);
	CHECK(str == "");

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

	reader.readRaw(3, str);
	CHECK(str == "RAW");
}


ADD_TEST(BinaryReaderWriterTest, Native) {

	std::stringstream sstream;
	BinaryWriter writer(sstream);
	BinaryReader reader(sstream);
	Write(writer);
	Read(reader);
}

ADD_TEST(BinaryReaderWriterTest, BigEndian) {

	std::stringstream sstream;
    BinaryWriter writer(sstream, BinaryWriter::BIG_ENDIAN_ORDER);
    BinaryReader reader(sstream, BinaryReader::BIG_ENDIAN_ORDER);
	
	Write(writer);
	Read(reader);
}

ADD_TEST(BinaryReaderWriterTest, LittleEndian) {

	std::stringstream sstream;
    BinaryWriter writer(sstream, BinaryWriter::LITTLE_ENDIAN_ORDER);
    BinaryReader reader(sstream, BinaryReader::LITTLE_ENDIAN_ORDER);
	
	Write(writer);
	Read(reader);
}
