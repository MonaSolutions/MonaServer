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
#include "Mona/String.h"
#include "Mona/Exceptions.h"
#include "Mona/Logs.h"
#include "math.h"
#include "float.h"

using namespace Mona;

using namespace std;

template<typename T>
bool tryToNumber(const std::string& value, T expected) { return tryToNumber<T>(value.c_str(), expected); }


template<typename T>
bool tryToNumber(const char * value, T expected) {

	Exception ex;
	T result = String::ToNumber<T>(ex, value);
	if (ex || result != expected)
		return false;
	return true;
}

/// \brief Use FLT_EPSILON by default
template<>
bool tryToNumber<float>(const char * value, float expected) {
	Exception ex;
	float result = String::ToNumber<float>(ex, value);
	if (ex || fabs(result - expected) > FLT_EPSILON)
		return false;
	return true;
}

template<>
bool tryToNumber<double>(const char * value, double expected) {
	Exception ex;
	double result = String::ToNumber<double>(ex, value);
	if (ex || fabs(result - expected) > DBL_EPSILON)
		return false;
	return true;
}



ADD_TEST(StringTest, General) {

	for (signed char c=0; c >= -10; ++c)
		CHECK((isalpha(c) && ((c>='a' && c<='z') || (c>='A' && c<='Z'))) || c < 'A' || (c > 'Z' && c<'a') || c>'z')

	for (signed char c=0; c >= -10; ++c)
		CHECK((isdigit(c) && c>='0' && c<='9') || c < '0' || c > '9')

	for (signed char c=0; c >= -10; ++c)
		CHECK((isalnum(c) && (isalpha(c) || isdigit(c))) || (!isalpha(c) && !isdigit(c)))

	for (signed char c=0; c >= -10; ++c)
		CHECK((isblank(c) && (c==' ' || c=='\t')) || (c!=' ' && c!='\t'))

	for (signed char c=0; c >= -10; ++c)
		CHECK((isspace(c) && ((c>='\t' && c<='\r') || c==' ')) || (c < '\t' || (c > '\r' && c!=' ')))

	for (signed char c=0; c >= -10; ++c)
		CHECK((iscntrl(c) && (c<=0x1F || c==0x7F)) || (c>0x1F && c!=0x7F))

	for (signed char c=0; c >= -10; ++c)
		CHECK((isgraph(c) && c>='!' && c<='~') || c < '!' || c > '~')

	for (signed char c=0; c >= -10; ++c)
		CHECK((islower(c) && c>='a' && c<='z') || c < 'a' || c > 'z')

	for (signed char c=0; c >= -10; ++c)
		CHECK((isupper(c) && c>='A' && c<='Z') || c < 'A' || c > 'Z')

	for (signed char c=0; c >= -10; ++c)
		CHECK((isprint(c) && c>=' ' && c<='~') || c < ' ' || c > '~')

	for (signed char c=0; c >= -10; ++c)
		CHECK((isxdigit(c) && ((c>='A' && c<='F') || (c>='a' && c<='f') || isdigit(c))) || c<'0' || (c > '9' && c<'A') || (c > 'F' && c<'a') || c>'f')

	for (signed char c=0; c >= -10; ++c)
		CHECK(tolower(c)==(c+32) || c < 'A' || c > 'Z')

	for (signed char c=0; c >= -10; ++c)
		CHECK(toupper(c)==(c-32) || c < 'a' || c > 'z')
	
}

string _Str;

ADD_TEST(StringTest, TestFormat) {

	CHECK(String::Format(_Str, 123) == "123");
	CHECK(String::Format(_Str, -123) == "-123");
	CHECK(String::Format(_Str, Format<int>("%5d", -123)) == " -123");

	CHECK(String::Format(_Str, (unsigned int)123) == "123");
	CHECK(String::Format(_Str, Format<unsigned int>("%5d", 123)) == "  123");
	CHECK(String::Format(_Str, Format<unsigned int>("%05d", 123)) == "00123");

	CHECK(String::Format(_Str, (long)123) == "123");
	CHECK(String::Format(_Str, (long)-123) == "-123");
	CHECK(String::Format(_Str, Format<long>("%5ld", -123)) == " -123");

	CHECK(String::Format(_Str, (unsigned long)123) == "123");
	CHECK(String::Format(_Str, Format<unsigned long>("%5lu", 123)) == "  123");

	CHECK(String::Format(_Str, (Int64)123) == "123");
	CHECK(String::Format(_Str, (Int64)-123) == "-123");
	CHECK(String::Format(_Str, Format<Int64>("%5lld", -123)) == " -123");

	CHECK(String::Format(_Str, (UInt64)123) == "123");
	CHECK(String::Format(_Str, Format<UInt64>("%5llu", 123)) == "  123");

	CHECK(String::Format(_Str, 12.25) == "12.25");
	CHECK(String::Format(_Str, Format<double>("%.4f", 12.25)) == "12.2500");
	CHECK(String::Format(_Str, Format<double>("%8.4f", 12.25)) == " 12.2500");
}


ADD_TEST(StringTest, TestFormat0) {

	CHECK(String::Format(_Str, Format<int>("%05d", 123)) == "00123");
	CHECK(String::Format(_Str, Format<int>("%05d", -123)) == "-0123");
	CHECK(String::Format(_Str, Format<long>("%05d", 123)) == "00123");
	CHECK(String::Format(_Str, Format<long>("%05d", -123)) == "-0123");
	CHECK(String::Format(_Str, Format<unsigned long>("%05d", 123)) == "00123");

	CHECK(String::Format(_Str, Format<Int64>("%05lld", 123)) == "00123");
	CHECK(String::Format(_Str, Format<Int64>("%05lld", -123)) == "-0123");
	CHECK(String::Format(_Str, Format<UInt64>("%05llu", 123)) == "00123");
}

ADD_TEST(StringTest, TestFloat) {

	string s(String::Format(_Str, 1.0f));
	CHECK(s == "1");
	s = String::Format(_Str, 0.1f);
	CHECK(s == "0.1");

	s = String::Format(_Str, 1.0);
	CHECK(s == "1");
	s = String::Format(_Str, 0.1);
	CHECK(s == "0.1");
}

ADD_TEST(StringTest, TestMultiple) {

	double pi = 3.1415926535897;
	CHECK(String::Format(_Str, "Pi ~= ", Format<double>("%.2f", pi), " (", pi, ")") == "Pi ~= 3.14 (3.1415926535897)");
	float pif = (float)3.1415926535897;
	CHECK(String::Format(_Str, "Pi ~= ", Format<double>("%.2f", pi), " (", pif, ")") == "Pi ~= 3.14 (3.1415927)");
	CHECK(String::Format(_Str, 18.0, "*", -2.0, " = ", 18.0*-2.0) == "18*-2 = -36");


    double big = 1234567890123456789.1234567890;
    CHECK(String::Format(_Str, big) == "1.234567890123457e+18");
	
    float bigf = 1234567890123456789.1234567890f;
    CHECK(String::Format(_Str, bigf) == "1.2345679e+18");

	// TODO
    //CHECK(tryToNumber<float>(_Str, bigf));
}

ADD_TEST(StringTest, ToNumber) {

	double value1(12.123456789);
	string format;
	String::Format(format, value1);
	double result1;
	String::ToNumber<double>(format, result1);
	CHECK(result1 == value1)

	float value2(12.123456789f);
	String::Format(format, value2);
	float result2;
	String::ToNumber<float>(format, result2);
	CHECK(result2 == value2)

    CHECK(tryToNumber<int>("123", 123));
    CHECK(tryToNumber<int>("-123", -123));
    CHECK(tryToNumber<UInt64>("123", 123));
    CHECK(!tryToNumber<UInt64>("-123", 123));
    CHECK(tryToNumber<double>("12.34", 12.34));
    CHECK(tryToNumber<float>("12.34", 12.34f));

	// Test of delta epsilon precision
    CHECK(tryToNumber<double>("0", DBL_EPSILON));
    CHECK(tryToNumber<float>("0", FLT_EPSILON));
    CHECK(!tryToNumber<double>("0", DBL_EPSILON * 2));
    CHECK(!tryToNumber<float>("0", FLT_EPSILON * 2));

    CHECK(!tryToNumber<double>("", 0));
    CHECK(!tryToNumber<double>("asd", 0));
    CHECK(!tryToNumber<double>("a123", 0));
    CHECK(!tryToNumber<double>("a123", 0));
    CHECK(!tryToNumber<double>("z23", 0));
    CHECK(!tryToNumber<double>("23z", 0));
    CHECK(!tryToNumber<double>("a12.3", 0));
    CHECK(!tryToNumber<double>("12.3aa", 0));

	
}

ADD_TEST(StringTest, TrimLeft) {

	string s = "abc";
	CHECK(String::Trim(s, String::TRIM_LEFT) == "abc");
	
	s = " abc ";
	CHECK(String::Trim(s, String::TRIM_LEFT) == "abc ");
	
	s = "  ab c ";
	CHECK(String::Trim(s, String::TRIM_LEFT) == "ab c ");
}


ADD_TEST(StringTest, TrimRight) {

	string s = "abc";
	CHECK(String::Trim(s, String::TRIM_RIGHT) == "abc");
	
	s = " abc ";
	CHECK(String::Trim(s, String::TRIM_RIGHT) == " abc");

	s = "  ab c  ";
	CHECK(String::Trim(s, String::TRIM_RIGHT) == "  ab c");
}

ADD_TEST(StringTest, Trim) {

	string s = "abc";
	CHECK(String::Trim(s, String::TRIM_BOTH) == "abc");
	
	s = "abc ";
	CHECK(String::Trim(s, String::TRIM_BOTH) == "abc");
	
	s = "  ab c  ";
	CHECK(String::Trim(s, String::TRIM_BOTH) == "ab c");
}

ADD_TEST(StringTest, Split) {

	int test = 0;
	String::ForEach forEach([&test](UInt32 index,const char* value) {
		switch (test) {
			case 0:
				CHECK(strcmp(value, "bidule")==0)
				break;
			case 1: {
				if (index == 0)
					CHECK(strcmp(value, "abc ")==0)
				else if (index == 1)
					CHECK(strcmp(value, " def ")==0)
				else if (index==2)
					CHECK(strcmp(value, " ghi")==0)
				else
					CHECK(strcmp(value, "")==0)
				break;
			}
			case 2: {
				if (index == 0)
					CHECK(strcmp(value, "abc")==0)
				else if (index == 1)
					CHECK(strcmp(value, "def")==0)
				else
					CHECK(strcmp(value, "ghi")==0)
				break;
			}
			default:
				FATAL_ERROR("No expected Split ", test, " test")
		}
		return true;
	});

	string s = "bidule"; test = 0;
	CHECK(String::Split(s,"|",forEach) == 1);
	
	s = "abc | def | ghi"; test = 1;
	CHECK(String::Split(s,"|",forEach) == 3);

	s = "abc | def | ghi|"; test = 1;
	CHECK(String::Split(s,"|",forEach) == 4);
	test = 1;
	CHECK(String::Split(s,"|",forEach,String::SPLIT_IGNORE_EMPTY) == 3);
	test = 2;
	CHECK(String::Split(s,"|",forEach,String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM) == 3);
}

ADD_TEST(StringTest, ToLower) {

	string s = "ABC";
	CHECK(String::ToLower(s) == "abc");
	
	s = "aBC";
	CHECK(String::ToLower(s) == "abc");
}

ADD_TEST(StringTest, ICompare) {

	string s1 = "AAA";
	string s2 = "aaa";
	string s3 = "bbb";
	string s4 = "cCcCc";
	string s5;
	CHECK(String::ICompare(s1, s2) == 0);
	CHECK(String::ICompare(s1, s3) < 0);
	CHECK(String::ICompare(s1, s4) < 0);
	CHECK(String::ICompare(s3, s1) > 0);
	CHECK(String::ICompare(s4, s2) > 0);
	CHECK(String::ICompare(s2, s4) < 0);
	CHECK(String::ICompare(s1, s5) > 0);
	CHECK(String::ICompare(s5, s4) < 0);

	string ss1 = "AAAzz";
	string ss2 = "aaaX";
	string ss3 = "bbbX";
	CHECK(String::ICompare(ss1, ss2, 3) == 0);
	CHECK(String::ICompare(ss1, ss3, 3) < 0);
	
	CHECK(String::ICompare(s1, s2.c_str()) == 0);
	CHECK(String::ICompare(s1, s3.c_str()) < 0);
	CHECK(String::ICompare(s1, s4.c_str()) < 0);
	CHECK(String::ICompare(s3, s1.c_str()) > 0);
	CHECK(String::ICompare(s4, s2.c_str()) > 0);
	CHECK(String::ICompare(s2, s4.c_str()) < 0);
	CHECK(String::ICompare(s1, s5.c_str()) > 0);
	CHECK(String::ICompare(s5, s4.c_str()) < 0);
	
	CHECK(String::ICompare(ss1, "aaa", 3) == 0);
	CHECK(String::ICompare(ss1, "AAA", 3) == 0);
	CHECK(String::ICompare(ss1, "bb", 2) < 0);


	CHECK(String::ICompare("true salut", "true", 4)==0);
	CHECK(String::ICompare("true salut", "true", 10)>0);
}
