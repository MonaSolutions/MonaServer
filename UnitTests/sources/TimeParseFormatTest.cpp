
#include "Test.h"
#include "Mona/Time.h"
#include "Mona/TimeParser.h"
#include "Mona/Logs.h"

using namespace Mona;
using namespace std;

string		_Out;
Time		_Time;

string& ToString(const string& fmt, int tzd = Time::UTC) {
	_Out.clear();
	_Time.toString(fmt, _Out, tzd);
	return _Out;
}


bool VerifyParsing(const char * stDate, int year, int month,
	int day, int hour, int min, int sec, int msec, int microsec, struct tm& tmdate) {

		// Conversion to struct tm
		_Time.toGMT(tmdate);

		bool bIsParseOk = (tmdate.tm_year + 1900) == year;
		if (bIsParseOk) bIsParseOk = (tmdate.tm_mon + 1) == month;
		if (bIsParseOk) bIsParseOk = tmdate.tm_mday == day;
		if (bIsParseOk) bIsParseOk = tmdate.tm_hour == hour;
		if (bIsParseOk) bIsParseOk = tmdate.tm_min == min;
		if (bIsParseOk) bIsParseOk = tmdate.tm_sec == sec;
		if (bIsParseOk) bIsParseOk = _Time.millisec() == msec;
		if (bIsParseOk) bIsParseOk = _Time.microsec() == microsec;

		return bIsParseOk;
}


bool IsParseOk(const char * fmt, const char * stDate, int year, int month,
	int day, int hour, int min, int sec, int msec, int microsec) {

	// Parsing
	int tzd = 0;
	bool bIsParseOk = TimeParser::Parse(fmt, stDate, _Time, tzd);
	if (!bIsParseOk) {
		DEBUG("Error during parsing of date (", stDate, ")");
		return false;
	}

	struct tm tmdate;
	if (VerifyParsing(stDate, year, month, day, hour, min, sec, msec, microsec, tmdate))
		return true;
	else {
		DEBUG((tmdate.tm_year + 1900),",",(tmdate.tm_mon + 1),",",tmdate.tm_mday,",",tmdate.tm_hour
			,",", tmdate.tm_min, ",", tmdate.tm_sec, ",", _Time.millisec(), ",", _Time.microsec(), " does not correspond to ", stDate, " (", fmt, ")");
		return false;
	}
}

bool IsParseOk(const char * stDate, int year, int month, 
	int day, int hour, int min, int sec, int msec, int microsec) {

	// Parsing
	bool bIsParseOk = _Time.fromString(stDate);
	if (!bIsParseOk) {
		DEBUG("Error during parsing of date (", stDate, ")");
		return false;
	}

	struct tm tmdate;
	if (VerifyParsing(stDate, year, month, day, hour, min, sec, msec, microsec, tmdate))
		return true;
	else {
		DEBUG((tmdate.tm_year + 1900), ",", (tmdate.tm_mon + 1), ",", tmdate.tm_mday, ",", tmdate.tm_hour
			,",", tmdate.tm_min, ",", tmdate.tm_sec, ",", _Time.millisec(), ",", _Time.microsec(), " does not correspond to ", stDate);
		return false;
	}
}

ADD_TEST(TimeParseFormatTest, TestISO8601) {

	CHECK(IsParseOk("2005-01-08T12:30:00Z", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::ISO8601_FORMAT) == "2005-01-08T12:30:00Z");

	CHECK(IsParseOk("2005-01-08T12:30:00+01:00", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::ISO8601_FORMAT, 3600) == "2005-01-08T12:30:00+01:00");

	CHECK(IsParseOk("2005-01-08T12:30:00-01:00", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::ISO8601_FORMAT, -3600) == "2005-01-08T12:30:00-01:00");

	CHECK(IsParseOk("2005-01-08T12:30:00", 2005, 1, 8, 12, 30, 0, 0, 0));
	
	CHECK(IsParseOk("2005-01-08", 2005, 1, 8, 0, 0, 0, 0, 0));
}

ADD_TEST(TimeParseFormatTest, TestISO8601Frac) {
	
	CHECK(IsParseOk("2005-01-08T12:30:00.1Z", 2005, 1, 8, 12, 30, 0, 100, 0));
	
	CHECK(IsParseOk("2005-01-08T12:30:00.123+01:00", 2005, 1, 8, 12, 30, 0, 123, 0));
	
	CHECK(IsParseOk("2005-01-08T12:30:00.12345-01:00", 2005, 1, 8, 12, 30, 0, 123, 450));
	
	CHECK(IsParseOk("2010-09-23T16:17:01.2817002+02:00", 2010, 9, 23, 16, 17, 1, 281, 700));
	
	CHECK(IsParseOk("2005-01-08T12:30:00.123456", 2005, 1, 8, 12, 30, 0, 123, 456));

	CHECK(IsParseOk("2005-01-08T12:30:00.012034", 2005, 1, 8, 12, 30, 0, 12, 34));
	CHECK(ToString(Time::ISO8601_FRAC_FORMAT, 3600) == "2005-01-08T12:30:00.012034+01:00");
	CHECK(ToString(Time::ISO8601_FRAC_FORMAT, -3600) == "2005-01-08T12:30:00.012034-01:00");
}


ADD_TEST(TimeParseFormatTest, TestRFC822) {

	CHECK(IsParseOk("Sat, 8 Jan 05 12:30:00 GMT", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC822_FORMAT) == "Sat, 8 Jan 05 12:30:00 GMT");

	CHECK(IsParseOk("Sat, 8 Jan 05 12:30:00 +0100", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC822_FORMAT, 3600) == "Sat, 8 Jan 05 12:30:00 +0100");

	CHECK(IsParseOk("Sat, 8 Jan 05 12:30:00 -0100", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC822_FORMAT, -3600) == "Sat, 8 Jan 05 12:30:00 -0100");

	CHECK(IsParseOk("Tue, 18 Jan 05 12:30:00 CET", 2005, 1, 18, 12, 30, 0, 0, 0));

	CHECK(IsParseOk("Wed, 12 Sep 73 02:01:12 CEST", 1973, 9, 12, 2, 1, 12, 0, 0));

	CHECK(!IsParseOk("12 Sep 73 02:01:12 CEST", 1973, 9, 12, 2, 1, 12, 0, 0));
	CHECK(IsParseOk(Time::RFC822_FORMAT.c_str() , "12 Sep 73 02:01:12 CEST", 1973, 9, 12, 2, 1, 12, 0, 0));
}


ADD_TEST(TimeParseFormatTest, TestRFC1123) {

	CHECK(IsParseOk("Sat, 8 Jan 2005 12:30:00 GMT", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC1123_FORMAT) == "Sat, 8 Jan 2005 12:30:00 GMT");

	CHECK(IsParseOk("Sat, 8 Jan 2005 12:30:00 +0100", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC1123_FORMAT, 3600) == "Sat, 8 Jan 2005 12:30:00 +0100");

	CHECK(IsParseOk("Sat, 8 Jan 2005 12:30:00 -0100", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC1123_FORMAT, -3600) == "Sat, 8 Jan 2005 12:30:00 -0100");

	CHECK(!IsParseOk("Sun, 20 Jul 1969 16:17:30 EDT", 1969, 7, 20, 16, 17, 30, 0, 0)); // Year invalid

	CHECK(!IsParseOk("Sun, 20 Jul 1969 16:17:30 GMT+01:00", 1969, 7, 20, 16, 17, 30, 0, 0));  // Year invalid
}


ADD_TEST(TimeParseFormatTest, TestHTTP) {

	CHECK(IsParseOk("Sat, 08 Jan 2005 12:30:00 GMT", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::HTTP_FORMAT) == "Sat, 08 Jan 2005 12:30:00 GMT");

	CHECK(IsParseOk("Sat, 08 Jan 2005 12:30:00 +0100", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::HTTP_FORMAT, 3600) == "Sat, 08 Jan 2005 12:30:00 +0100");

	CHECK(IsParseOk("Sat, 08 Jan 2005 12:30:00 -0100", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::HTTP_FORMAT, -3600) == "Sat, 08 Jan 2005 12:30:00 -0100");
}


ADD_TEST(TimeParseFormatTest, TestRFC850) {

	CHECK(IsParseOk("Saturday, 8-Jan-05 12:30:00 GMT", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC850_FORMAT) == "Saturday, 8-Jan-05 12:30:00 GMT");

	CHECK(IsParseOk("Saturday, 8-Jan-05 12:30:00 +0100", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC850_FORMAT, 3600) == "Saturday, 8-Jan-05 12:30:00 +0100");

	CHECK(IsParseOk("Saturday, 8-Jan-05 12:30:00 -0100", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC850_FORMAT, -3600) == "Saturday, 8-Jan-05 12:30:00 -0100");

	CHECK(IsParseOk("Wed, 12-Sep-73 02:01:12 CEST", 1973, 9, 12, 2, 1, 12, 0, 0));
}


ADD_TEST(TimeParseFormatTest, TestRFC1036) {

	CHECK(IsParseOk("Saturday, 8 Jan 05 12:30:00 GMT", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC1036_FORMAT) == "Saturday, 8 Jan 05 12:30:00 GMT");

	CHECK(IsParseOk("Saturday, 8 Jan 05 12:30:00 +0100", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC1036_FORMAT, 3600) == "Saturday, 8 Jan 05 12:30:00 +0100");

	CHECK(IsParseOk("Saturday, 8 Jan 05 12:30:00 -0100", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::RFC1036_FORMAT, -3600) == "Saturday, 8 Jan 05 12:30:00 -0100");
}


ADD_TEST(TimeParseFormatTest, TestASCTIME) {

	CHECK(IsParseOk("Sat Jan  8 12:30:00 2005", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::ASCTIME_FORMAT) == "Sat Jan  8 12:30:00 2005");
}


ADD_TEST(TimeParseFormatTest, TestSORTABLE) {

	CHECK(IsParseOk("2005-01-08 12:30:00", 2005, 1, 8, 12, 30, 0, 0, 0));
	CHECK(ToString(Time::SORTABLE_FORMAT) == "2005-01-08 12:30:00");
}


ADD_TEST(TimeParseFormatTest, TestCustom){

	CHECK(IsParseOk("%d-%b-%Y", "18-Jan-2005", 2005, 1, 18, 0, 0, 0, 0, 0));
	
	CHECK(IsParseOk("%m/%d/%y", "01/18/05", 2005, 1, 18, 0, 0, 0, 0, 0));

	CHECK(IsParseOk("%h:%M %a", "12:30 am", 1970, 1, 1, 0, 30, 0, 0, 0));

	CHECK(IsParseOk("%h:%M %a", "12:30 PM", 1970, 1, 1, 12, 30, 0, 0, 0));

	CHECK(IsParseOk("%w/%B/%d/%Y/%h/%A/%M/%S/%F/%Z/%%",
		"Sat/January/08/2005/12/PM/30/00/250000/GMT/%", 2005, 1, 8, 12, 30, 0, 250, 0));
	CHECK(ToString("%w/%W/%b/%B/%d/%e/%f/%m/%n/%o/%y/%Y/%H/%h/%a/%A/%M/%S/%i/%c/%z/%Z/%%") == 
		"Sat/Saturday/Jan/January/08/8/ 8/01/1/ 1/05/2005/12/12/pm/PM/30/00/250/2/Z/GMT/%");
}


ADD_TEST(TimeParseFormatTest, TestGuess) {

	CHECK(IsParseOk("2005-01-08T12:30:00Z", 2005, 1, 8, 12, 30, 0, 0, 0));

	CHECK(IsParseOk("20050108T123000Z", 2005, 1, 8, 12, 30, 0, 0, 0));

	CHECK(IsParseOk("2005-01-08T12:30:00+01:00", 2005, 1, 8, 12, 30, 0, 0, 0));

	CHECK(IsParseOk("2005-01-08T12:30:00.123456Z", 2005, 1, 8, 12, 30, 0, 123, 456));

	CHECK(IsParseOk("2005-01-08T12:30:00,123456Z", 2005, 1, 8, 12, 30, 0, 123, 456));

	CHECK(IsParseOk("20050108T123000,123456Z", 2005, 1, 8, 12, 30, 0, 123, 456));

	CHECK(IsParseOk("20050108T123000.123+0200", 2005, 1, 8, 12, 30, 0, 123, 0));

	CHECK(IsParseOk("2005-01-08T12:30:00.123456-02:00", 2005, 1, 8, 12, 30, 0, 123, 456));
}
