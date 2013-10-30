
#include "StringTest.h"
#include "Mona/String.h"
#include "Mona/Exceptions.h"

using namespace Mona;

using namespace std;

ADD_TEST(StringTest, TestFormat) {

	EXPECT_TRUE(String::Format(str, 123) == "123");
	EXPECT_TRUE(String::Format(str, -123) == "-123");
	EXPECT_TRUE(String::Format(str, Format<int>("%5d", -123)) == " -123");

	EXPECT_TRUE(String::Format(str, (unsigned int)123) == "123");
	EXPECT_TRUE(String::Format(str, Format<unsigned int>("%5d", 123)) == "  123");
	EXPECT_TRUE(String::Format(str, Format<unsigned int>("%05d", 123)) == "00123");

	EXPECT_TRUE(String::Format(str, (long)123) == "123");
	EXPECT_TRUE(String::Format(str, (long)-123) == "-123");
	EXPECT_TRUE(String::Format(str, Format<long>("%5ld", -123)) == " -123");

	EXPECT_TRUE(String::Format(str, (unsigned long)123) == "123");
	EXPECT_TRUE(String::Format(str, Format<unsigned long>("%5lu", 123)) == "  123");

	EXPECT_TRUE(String::Format(str, (Int64)123) == "123");
	EXPECT_TRUE(String::Format(str, (Int64)-123) == "-123");
	EXPECT_TRUE(String::Format(str, Format<Int64>("%5" I64_FMT "d", -123)) == " -123");

	EXPECT_TRUE(String::Format(str, (UInt64)123) == "123");
	EXPECT_TRUE(String::Format(str, Format<UInt64>("%5" I64_FMT "u", 123)) == "  123");

	EXPECT_TRUE(String::Format(str, 12.25) == "12.25");
	EXPECT_TRUE(String::Format(str, Format<double>("%.4f", 12.25)) == "12.2500");
	EXPECT_TRUE(String::Format(str, Format<double>("%8.4f", 12.25)) == " 12.2500");
}


ADD_TEST(StringTest, TestFormat0) {

	EXPECT_TRUE(String::Format(str, Format<int>("%05d", 123)) == "00123");
	EXPECT_TRUE(String::Format(str, Format<int>("%05d", -123)) == "-0123");
	EXPECT_TRUE(String::Format(str, Format<long>("%05d", 123)) == "00123");
	EXPECT_TRUE(String::Format(str, Format<long>("%05d", -123)) == "-0123");
	EXPECT_TRUE(String::Format(str, Format<unsigned long>("%05d", 123)) == "00123");

	EXPECT_TRUE(String::Format(str, Format<Int64>("%05d", 123)) == "00123");
	EXPECT_TRUE(String::Format(str, Format<Int64>("%05d", -123)) == "-0123");
	EXPECT_TRUE(String::Format(str, Format<UInt64>("%05d", 123)) == "00123");
}

ADD_TEST(StringTest, TestFloat) {

	string s(String::Format(str, 1.0f));
	EXPECT_TRUE(s == "1");
	s = String::Format(str, 0.1f);
	EXPECT_TRUE(s == "0.1");

	s = String::Format(str, 1.0);
	EXPECT_TRUE(s == "1");
	s = String::Format(str, 0.1);
	EXPECT_TRUE(s == "0.1");
}

ADD_TEST(StringTest, TestMultiple) {

	double pi = 3.1415926535897;
	EXPECT_TRUE(String::Format(str, "Pi ~= ", Format<double>("%.2f", pi), " (", pi, ")") == "Pi ~= 3.14 (3.1415926535897)");
	float pif = (float)3.1415926535897;
	EXPECT_TRUE(String::Format(str, "Pi ~= ", Format<double>("%.2f", pi), " (", pif, ")") == "Pi ~= 3.14 (3.1415927)");
	EXPECT_TRUE(String::Format(str, 18.0, "*", -2.0, " = ", 18.0*-2.0) == "18*-2 = -36");


	double big = 1234567890123456789.1234567890;
	EXPECT_TRUE(String::Format(str, big) == "1.234567890123457e+018");
	
	float bigf = (float)1234567890123456789.1234567890;
	EXPECT_TRUE(String::Format(str, bigf) == "1.2345679e+018");

	// TODO
	//EXPECT_TRUE(StringTest::ToNumber<float>(str, bigf));
}

ADD_TEST(StringTest, TestToNumber) {

	EXPECT_TRUE(StringTest::ToNumber<int>("123", 123));
	EXPECT_TRUE(StringTest::ToNumber<int>("-123", -123));
	EXPECT_TRUE(StringTest::ToNumber<UInt64>("123", 123));
	EXPECT_TRUE(!StringTest::ToNumber<UInt64>("-123", 123));
	EXPECT_TRUE(StringTest::ToNumber<double>("12.34", 12.34));
	EXPECT_TRUE(StringTest::ToNumber<float>("12.34", 12.34f));

	// Test of delta epsilon precision
	EXPECT_TRUE(StringTest::ToNumber<double>("0", DBL_EPSILON));
	EXPECT_TRUE(StringTest::ToNumber<float>("0", FLT_EPSILON));
	EXPECT_TRUE(!StringTest::ToNumber<double>("0", DBL_EPSILON * 2));
	EXPECT_TRUE(!StringTest::ToNumber<float>("0", FLT_EPSILON * 2));

	EXPECT_TRUE(!StringTest::ToNumber<double>("", 0));
	EXPECT_TRUE(!StringTest::ToNumber<double>("asd", 0));
	EXPECT_TRUE(!StringTest::ToNumber<double>("a123", 0));
	EXPECT_TRUE(!StringTest::ToNumber<double>("a123", 0));
	EXPECT_TRUE(!StringTest::ToNumber<double>("z23", 0));
	EXPECT_TRUE(!StringTest::ToNumber<double>("23z", 0));
	EXPECT_TRUE(!StringTest::ToNumber<double>("a12.3", 0));
	EXPECT_TRUE(!StringTest::ToNumber<double>("12.3aa", 0));
}

ADD_TEST(StringTest, TrimLeft) {

	string s = "abc";
	EXPECT_TRUE(String::Trim(s, String::TRIM_LEFT) == "abc");
	
	s = " abc ";
	EXPECT_TRUE(String::Trim(s, String::TRIM_LEFT) == "abc ");
	
	s = "  ab c ";
	EXPECT_TRUE(String::Trim(s, String::TRIM_LEFT) == "ab c ");
}


ADD_TEST(StringTest, TrimRight) {

	string s = "abc";
	EXPECT_TRUE(String::Trim(s, String::TRIM_RIGHT) == "abc");
	
	s = " abc ";
	EXPECT_TRUE(String::Trim(s, String::TRIM_RIGHT) == " abc");

	s = "  ab c  ";
	EXPECT_TRUE(String::Trim(s, String::TRIM_RIGHT) == "  ab c");
}

ADD_TEST(StringTest, Trim) {

	string s = "abc";
	EXPECT_TRUE(String::Trim(s, String::TRIM_BOTH) == "abc");
	
	s = "abc ";
	EXPECT_TRUE(String::Trim(s, String::TRIM_BOTH) == "abc");
	
	s = "  ab c  ";
	EXPECT_TRUE(String::Trim(s, String::TRIM_BOTH) == "ab c");
}

ADD_TEST(StringTest, ToLower) {

	string s = "ABC";
	EXPECT_TRUE(String::ToLower(s) == "abc");
	
	s = "aBC";
	EXPECT_TRUE(String::ToLower(s) == "abc");
}

ADD_TEST(StringTest, ICompare) {

	string s1 = "AAA";
	string s2 = "aaa";
	string s3 = "bbb";
	string s4 = "cCcCc";
	string s5;
	EXPECT_TRUE(String::ICompare(s1, s2) == 0);
	EXPECT_TRUE(String::ICompare(s1, s3) < 0);
	EXPECT_TRUE(String::ICompare(s1, s4) < 0);
	EXPECT_TRUE(String::ICompare(s3, s1) > 0);
	EXPECT_TRUE(String::ICompare(s4, s2) > 0);
	EXPECT_TRUE(String::ICompare(s2, s4) < 0);
	EXPECT_TRUE(String::ICompare(s1, s5) > 0);
	EXPECT_TRUE(String::ICompare(s5, s4) < 0);

	string ss1 = "AAAzz";
	string ss2 = "aaaX";
	string ss3 = "bbbX";
	EXPECT_TRUE(String::ICompare(ss1, ss2, 3) == 0);
	EXPECT_TRUE(String::ICompare(ss1, ss3, 3) < 0);
	
	EXPECT_TRUE(String::ICompare(s1, s2.c_str()) == 0);
	EXPECT_TRUE(String::ICompare(s1, s3.c_str()) < 0);
	EXPECT_TRUE(String::ICompare(s1, s4.c_str()) < 0);
	EXPECT_TRUE(String::ICompare(s3, s1.c_str()) > 0);
	EXPECT_TRUE(String::ICompare(s4, s2.c_str()) > 0);
	EXPECT_TRUE(String::ICompare(s2, s4.c_str()) < 0);
	EXPECT_TRUE(String::ICompare(s1, s5.c_str()) > 0);
	EXPECT_TRUE(String::ICompare(s5, s4.c_str()) < 0);
	
	EXPECT_TRUE(String::ICompare(ss1, "aaa", 3) == 0);
	EXPECT_TRUE(String::ICompare(ss1, "AAA", 3) == 0);
	EXPECT_TRUE(String::ICompare(ss1, "bb", 2) < 0);
}
