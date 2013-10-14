
#include "StringTest.h"
#include "Mona/String.h"

using namespace Mona;

using namespace std;

TEST_F(StringTest, TestFormat) {

	EXPECT_EQ(String::Format(123), "123");
	EXPECT_EQ(String::Format(-123), "-123");
	EXPECT_EQ(String::Format(pair<const char *, int>("%5d", -123)), " -123");

	EXPECT_EQ(String::Format((unsigned int)123), "123");
	EXPECT_EQ(String::Format(pair<const char *, unsigned int>("%5d", 123)), "  123");
	EXPECT_EQ(String::Format(pair<const char *, unsigned int>("%05d", 123)), "00123");

	EXPECT_EQ(String::Format((long)123), "123");
	EXPECT_EQ(String::Format((long)-123), "-123");
	EXPECT_EQ(String::Format(pair<const char *, long>("%5ld", -123)), " -123");

	EXPECT_EQ(String::Format((unsigned long)123), "123");
	EXPECT_EQ(String::Format(pair<const char *, unsigned long>("%5lu", 123)), "  123");

	EXPECT_EQ(String::Format((Int64)123), "123");
	EXPECT_EQ(String::Format((Int64)-123), "-123");
	EXPECT_EQ(String::Format(pair<const char *, Int64>("%5" I64_FMT "d", -123)), " -123");

	EXPECT_EQ(String::Format((UInt64)123), "123");
	EXPECT_EQ(String::Format(pair<const char *, UInt64>("%5" I64_FMT "u", 123)), "  123");

	EXPECT_EQ(String::Format(12.25), "12.25");
	EXPECT_EQ(String::Format(pair<const char *, double>("%.4f", 12.25)), "12.2500");
	EXPECT_EQ(String::Format(pair<const char *, double>("%8.4f", 12.25)), " 12.2500");
}


TEST_F(StringTest, TestFormat0) {

	EXPECT_EQ(String::Format(pair<const char *, int>("%05d", 123)), "00123");
	EXPECT_EQ(String::Format(pair<const char *, int>("%05d", -123)), "-0123");
	EXPECT_EQ(String::Format(pair<const char *, long>("%05d", 123)), "00123");
	EXPECT_EQ(String::Format(pair<const char *, long>("%05d", -123)), "-0123");
	EXPECT_EQ(String::Format(pair<const char *, unsigned long>("%05d", 123)), "00123");

	EXPECT_EQ(String::Format(pair<const char *, Int64>("%05d", 123)), "00123");
	EXPECT_EQ(String::Format(pair<const char *, Int64>("%05d", -123)), "-0123");
	EXPECT_EQ(String::Format(pair<const char *, UInt64>("%05d", 123)), "00123");
}

TEST_F(StringTest, TestFloat) {

	std::string s(String::Format(1.0f));
	EXPECT_EQ(s, "1");
	s = String::Format(0.1f);
	EXPECT_EQ(s, "0.1");

	s = String::Format(1.0);
	EXPECT_EQ(s, "1");
	s = String::Format(0.1);
	EXPECT_EQ(s, "0.1");
}

TEST_F(StringTest, TestMultiple) {

	double pi = 3.1415926535897;
	EXPECT_EQ(String::Format("Pi ~= ", pair<const char *, double>("%.2f", pi), " (", pi, ")"), "Pi ~= 3.14 (3.14159)");
	EXPECT_EQ(String::Format(18.0, "*", -2.0, " = ", 18.0*-2.0), "18*-2 = -36");
}
