#include "Test.h"
#include "Mona/BinaryWriter.h"
#include "Mona/BinaryReader.h"
#include <sstream>

using namespace Mona;
using namespace std;



void Write(BinaryWriter& writer) {

	bool bval = true;
	writer.writeRaw((char*)&bval, sizeof(bval));
	bval = false;
	writer.writeRaw((char*)&bval, sizeof(bval));
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

	bool b;
	reader.readRaw((char*)&b, sizeof(b));
	EXPECT_TRUE(b);
	reader.readRaw((char*)&b, sizeof(b));
	EXPECT_TRUE(!b);

	char c = reader.read8();
	EXPECT_TRUE(c == 'a');

	short shortv = reader.read16();
	EXPECT_TRUE(shortv == -100);

	unsigned short ushortv = reader.read16();
	EXPECT_TRUE(ushortv == 50000);

	int intv = reader.read32();
	EXPECT_TRUE(intv == -123456);

	unsigned uintv = reader.read32();
	EXPECT_TRUE(uintv == 123456);

	long longv = reader.read32();
	EXPECT_TRUE(longv == -1234567890);

	unsigned long ulongv = reader.read32();
	EXPECT_TRUE(ulongv == 1234567890);

	Int64 int64v = reader.read64();
	EXPECT_TRUE(int64v == -1234567890);

	UInt64 uint64v = reader.read64();
	EXPECT_TRUE(uint64v == 1234567890);

	float floatv;
	reader.readRaw((char *)&floatv, sizeof(floatv));
	EXPECT_TRUE(floatv == 1.5);

	double doublev;
	reader.readRaw((char *)&doublev, sizeof(doublev));
	EXPECT_TRUE(doublev == -1.5);

	std::string str;
	reader.readString8(str);
	EXPECT_TRUE(str == "foo");
	reader.readString8(str);
	EXPECT_TRUE(str == "");
	reader.readString8(str);
	EXPECT_TRUE(str == "bar");
	reader.readString8(str);
	EXPECT_TRUE(str == "");

	UInt32 uint32v = reader.read7BitValue();
	EXPECT_TRUE(uint32v == 100);
	uint32v = reader.read7BitValue();
	EXPECT_TRUE(uint32v == 1000);
	uint32v = reader.read7BitValue();
	EXPECT_TRUE(uint32v == 10000);
	uint32v = reader.read7BitValue();
	EXPECT_TRUE(uint32v == 100000);
	uint32v = reader.read7BitValue();
	EXPECT_TRUE(uint32v == 1000000);

	uint64v = reader.read7BitLongValue();
	EXPECT_TRUE(uint64v == 100);
	uint64v = reader.read7BitLongValue();
	EXPECT_TRUE(uint64v == 1000);
	uint64v = reader.read7BitLongValue();
	EXPECT_TRUE(uint64v == 10000);
	uint64v = reader.read7BitLongValue();
	EXPECT_TRUE(uint64v == 100000);
	uint64v = reader.read7BitLongValue();
	EXPECT_TRUE(uint64v == 1000000);

	reader.readRaw(3, str);
	EXPECT_TRUE(str == "RAW");
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
	BinaryWriter writer(sstream, BinaryWriter::BIG_ENDIAN);
	BinaryReader reader(sstream, BinaryReader::BIG_ENDIAN);
	
	Write(writer);
	Read(reader);
}

ADD_TEST(BinaryReaderWriterTest, LittleEndian) {

	std::stringstream sstream;
	BinaryWriter writer(sstream, BinaryWriter::LITTLE_ENDIAN);
	BinaryReader reader(sstream, BinaryReader::LITTLE_ENDIAN);
	
	Write(writer);
	Read(reader);
}
