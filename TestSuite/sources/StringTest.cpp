
#include "StringTest.h"
#include "Mona/String.h"

using namespace Mona;

using namespace std;

TEST_F(StringTest, TestFormat) {

	EXPECT_EQ(String::Format(str, 123), "123");
	EXPECT_EQ(String::Format(str, -123), "-123");
	EXPECT_EQ(String::Format(str, Format<int>("%5d", -123)), " -123");

	EXPECT_EQ(String::Format(str, (unsigned int)123), "123");
	EXPECT_EQ(String::Format(str, Format<unsigned int>("%5d", 123)), "  123");
	EXPECT_EQ(String::Format(str, Format<unsigned int>("%05d", 123)), "00123");

	EXPECT_EQ(String::Format(str, (long)123), "123");
	EXPECT_EQ(String::Format(str, (long)-123), "-123");
	EXPECT_EQ(String::Format(str, Format<long>("%5ld", -123)), " -123");

	EXPECT_EQ(String::Format(str, (unsigned long)123), "123");
	EXPECT_EQ(String::Format(str, Format<unsigned long>("%5lu", 123)), "  123");

	EXPECT_EQ(String::Format(str, (Int64)123), "123");
	EXPECT_EQ(String::Format(str, (Int64)-123), "-123");
	EXPECT_EQ(String::Format(str, Format<Int64>("%5" I64_FMT "d", -123)), " -123");

	EXPECT_EQ(String::Format(str, (UInt64)123), "123");
	EXPECT_EQ(String::Format(str, Format<UInt64>("%5" I64_FMT "u", 123)), "  123");

	EXPECT_EQ(String::Format(str, 12.25), "12.25");
	EXPECT_EQ(String::Format(str, Format<double>("%.4f", 12.25)), "12.2500");
	EXPECT_EQ(String::Format(str, Format<double>("%8.4f", 12.25)), " 12.2500");
}


TEST_F(StringTest, TestFormat0) {

	EXPECT_EQ(String::Format(str, Format<int>("%05d", 123)), "00123");
	EXPECT_EQ(String::Format(str, Format<int>("%05d", -123)), "-0123");
	EXPECT_EQ(String::Format(str, Format<long>("%05d", 123)), "00123");
	EXPECT_EQ(String::Format(str, Format<long>("%05d", -123)), "-0123");
	EXPECT_EQ(String::Format(str, Format<unsigned long>("%05d", 123)), "00123");

	EXPECT_EQ(String::Format(str, Format<Int64>("%05d", 123)), "00123");
	EXPECT_EQ(String::Format(str, Format<Int64>("%05d", -123)), "-0123");
	EXPECT_EQ(String::Format(str, Format<UInt64>("%05d", 123)), "00123");
}

TEST_F(StringTest, TestFloat) {

	std::string s(String::Format(str, 1.0f));
	EXPECT_EQ(s, "1");
	s = String::Format(str, 0.1f);
	EXPECT_EQ(s, "0.1");

	s = String::Format(str, 1.0);
	EXPECT_EQ(s, "1");
	s = String::Format(str, 0.1);
	EXPECT_EQ(s, "0.1");
}

TEST_F(StringTest, TestMultiple) {

	double pi = 3.1415926535897;
	EXPECT_EQ(String::Format(str, "Pi ~= ", Format<double>("%.2f", pi), " (", pi, ")"), "Pi ~= 3.14 (3.1415926535897)");
	float pif = (float)3.1415926535897;
	EXPECT_EQ(String::Format(str, "Pi ~= ", Format<double>("%.2f", pi), " (", pif, ")"), "Pi ~= 3.14 (3.1415927)");
	EXPECT_EQ(String::Format(str, 18.0, "*", -2.0, " = ", 18.0*-2.0), "18*-2 = -36");


	double big = 1234567890123456789.1234567890;
	EXPECT_EQ(String::Format(str, big), "1.234567890123457e+018");
	
	float bigf = (float)1234567890123456789.1234567890;
	EXPECT_EQ(String::Format(str, bigf), "1.2345679e+018");
}
