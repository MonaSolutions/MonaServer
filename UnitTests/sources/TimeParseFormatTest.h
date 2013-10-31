
#pragma once

#include "Test.h"
#include "Mona/Time.h"

// The fixture for testing class Foo.
class TimeParseFormatTest : public Test {
protected:
	// You can remove any or all of the following functions if its body
	// is empty.

	TimeParseFormatTest(const char * testName) : Test(testName) {
		// You can do set-up work for each test here.
	}

	virtual ~TimeParseFormatTest() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	/// Time.fromString(date)
	bool IsParseOk(const char * stDate, int year, int month,
		int day, int hour, int min, int sec, int msec, int microsec);

	/// TimeParser::parse(fmt, date)
	bool IsParseOk(const char * fmt, const char * stDate, int year, int month,
		int day, int hour, int min, int sec, int msec, int microsec) ;

	std::string& ToString(const std::string& fmt, int tzd = Mona::Time::UTC);

	// Objects declared here can be used by all tests in the test case for Foo.
private:
	bool VerifyParsing(const char * stDate, int year, int month,
		int day, int hour, int min, int sec, int msec, int microsec, struct tm& tmdate);

	Mona::Time _time;
	std::string _out;
};

