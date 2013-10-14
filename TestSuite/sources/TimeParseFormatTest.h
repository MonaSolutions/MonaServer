
#pragma once

#include "gtest/gtest.h"
#include "Mona/Time.h"

// The fixture for testing class Foo.
class TimeParseFormatTest : public ::testing::Test {
protected:
	// You can remove any or all of the following functions if its body
	// is empty.

	TimeParseFormatTest() {
		// You can do set-up work for each test here.
	}

	virtual ~TimeParseFormatTest() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp() {
		// Code here will be called immediately after the constructor (right
		// before each test).
	}

	virtual void TearDown() {
		// Code here will be called immediately after each test (right
		// before the destructor).
	}

	/// Time.fromString(date)
	static ::testing::AssertionResult IsParseOk(const char * stDate, int year, int month,
		int day, int hour, int min, int sec, int msec, int microsec);

	/// TimeParser::parse(fmt, date)
	::testing::AssertionResult IsParseOk(const char * fmt, const char * stDate, int year, int month,
		int day, int hour, int min, int sec, int msec, int microsec);

	std::string& ToString(const std::string& fmt, int tzd = Mona::Time::UTC);

	// Objects declared here can be used by all tests in the test case for Foo.
private:
	static bool VerifyParsing(const char * stDate, int year, int month,
		int day, int hour, int min, int sec, int msec, int microsec, struct tm& tmdate);

	static char * _datestring;
	static Mona::Time _time;
	std::string _out;
};

