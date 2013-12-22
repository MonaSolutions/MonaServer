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

#include "Mona/TimeParser.h"
#include "Mona/Time.h"


using namespace std;

namespace Mona {

#define SKIP_JUNK() \
	while (it != end && !isdigit(*it)) ++it

#define SKIP_DIGITS() \
	while (it != end && isdigit(*it)) ++it

#define PARSE_NUMBER(var) \
	while (it != end && isdigit(*it)) var = var * 10 + ((*it++) - '0')

#define PARSE_NUMBER_N(var, n) \
	{ int i = 0; while (i++ < n && it != end && isdigit(*it)) var = var * 10 + ((*it++) - '0');}

#define PARSE_FRACTIONAL_N(var, n) \
	{ int i = 0; while (i < n && it != end && isdigit(*it)) { var = var * 10 + ((*it++) - '0'); i++; } while (i++ < n) var *= 10; }

bool TimeParser::TryParse(const string& in, Time& dateTime, int& tz) {
	
	if (in.length() < 4) return false;

	if (in[3] == ',')
		return Parse("%w, %e %b %r %H:%M:%S %Z", in, dateTime, tz);
	if (in[3] == ' ')
		return Parse(Time::ASCTIME_FORMAT, in, dateTime, tz);
	if (in.find(',') < 10)
		return Parse("%W, %e %b %r %H:%M:%S %Z", in, dateTime, tz);

	if (!isdigit(in[0]))
		return false;

	if (in.find(' ') != string::npos || in.length() == 10)
		return Parse(Time::SORTABLE_FORMAT, in, dateTime, tz);
	if (in.find('.') != string::npos || in.find(',') != string::npos)
		return Parse(Time::ISO8601_FRAC_FORMAT, in, dateTime, tz);
	if (in.find('-') != string::npos)
		return Parse(Time::ISO8601_FORMAT, in, dateTime, tz);
	return false;
}

bool TimeParser::Parse(const string& fmt, const string& in, Time& dateTime, int& tzdifferencial) {

	int year = 0, month = 0, day = 0, hour = 0;
	int minute = 0, second = 0, millis = 0, micros = 0;

	auto it = in.begin(), end = in.end();
	auto itf = fmt.begin(),	endf = fmt.end();

	while (itf != endf && it != end)
	{
		if (*itf == '%')
		{
			if (++itf != endf)
			{
				switch (*itf)
				{
				case 'w':
				case 'W':
					while (it != end && isspace(*it)) ++it;
					while (it != end && isalpha(*it)) ++it;
					break;
				case 'b':
				case 'B':
					ParseMonth(it, end, month);
					break;
				case 'd':
				case 'e':
				case 'f':
					SKIP_JUNK();
					PARSE_NUMBER_N(day, 2);
					break;
				case 'm':
				case 'n':
				case 'o':
					SKIP_JUNK();
					PARSE_NUMBER_N(month, 2);
					break;
				case 'y':
					SKIP_JUNK();
					PARSE_NUMBER_N(year, 2);
					if (year >= 69)
						year += 1900;
					else
						year += 2000;
					break;
				case 'Y':
					SKIP_JUNK();
					PARSE_NUMBER_N(year, 4);
					break;
				case 'r':
					SKIP_JUNK();
					PARSE_NUMBER(year);
					if (year < 1000)
					{
						if (year >= 69)
							year += 1900;
						else
							year += 2000;
					}
					break;
				case 'H':
				case 'h':
					SKIP_JUNK();
					PARSE_NUMBER_N(hour, 2);
					break;
				case 'a':
				case 'A':
					ParseAMPM(it, end, hour);
					break;
				case 'M':
					SKIP_JUNK();
					PARSE_NUMBER_N(minute, 2);
					break;
				case 'S':
					SKIP_JUNK();
					PARSE_NUMBER_N(second, 2);
					break;
				case 's':
					SKIP_JUNK();
					PARSE_NUMBER_N(second, 2);
					if (it != end && (*it == '.' || *it == ','))
					{
						++it;
						PARSE_FRACTIONAL_N(millis, 3);
						PARSE_FRACTIONAL_N(micros, 3);
						SKIP_DIGITS();
					}
					break;
				case 'i':
					SKIP_JUNK();
					PARSE_NUMBER_N(millis, 3);
					break;
				case 'c':
					SKIP_JUNK();
					PARSE_NUMBER_N(millis, 1);
					millis *= 100;
					break;
				case 'F':
					SKIP_JUNK();
					PARSE_FRACTIONAL_N(millis, 3);
					PARSE_FRACTIONAL_N(micros, 3);
					SKIP_DIGITS();
					break;
				case 'z':
				case 'Z':
					ParseTZD(it, end, tzdifferencial);
					break;
				}
				++itf;
			}
		}
		else ++itf;
	}
	if (month == 0) month = 1;
	if (day == 0) day = 1;
	if (year < 1970) year = 1970;

	struct tm tmtime;
	tmtime.tm_year = year - 1900;
	tmtime.tm_mon = month - 1;
	tmtime.tm_mday = day;
	tmtime.tm_hour = hour;
	tmtime.tm_min = minute;
	tmtime.tm_sec = second;

	if (Time::IsValid(tmtime, millis, micros)) {
		dateTime.update(tmtime, millis, micros);
		return true;
	}
	return false;
}

void TimeParser::ParseTZD(string::const_iterator& it, const string::const_iterator& end, int& result) {

	struct Zone
	{
		const char* designator;
		int         tzdifferencial;
	};

	static Zone zones[] =
	{
		{ "Z", 0 },
		{ "UT", 0 },
		{ "GMT", 0 },
		{ "BST", 1 * 3600 },
		{ "IST", 1 * 3600 },
		{ "WET", 0 },
		{ "WEST", 1 * 3600 },
		{ "CET", 1 * 3600 },
		{ "CEST", 2 * 3600 },
		{ "EET", 2 * 3600 },
		{ "EEST", 3 * 3600 },
		{ "MSK", 3 * 3600 },
		{ "MSD", 4 * 3600 },
		{ "NST", -3 * 3600 - 1800 },
		{ "NDT", -2 * 3600 - 1800 },
		{ "AST", -4 * 3600 },
		{ "ADT", -3 * 3600 },
		{ "EST", -5 * 3600 },
		{ "EDT", -4 * 3600 },
		{ "CST", -6 * 3600 },
		{ "CDT", -5 * 3600 },
		{ "MST", -7 * 3600 },
		{ "MDT", -6 * 3600 },
		{ "PST", -8 * 3600 },
		{ "PDT", -7 * 3600 },
		{ "AKST", -9 * 3600 },
		{ "AKDT", -8 * 3600 },
		{ "HST", -10 * 3600 },
		{ "AEST", 10 * 3600 },
		{ "AEDT", 11 * 3600 },
		{ "ACST", 9 * 3600 + 1800 },
		{ "ACDT", 10 * 3600 + 1800 },
		{ "AWST", 8 * 3600 },
		{ "AWDT", 9 * 3600 }
	};

	int tzd = 0;
	while (it != end && isspace(*it)) ++it;
	if (it != end)
	{
		if (isalpha(*it))
		{
			string designator;
			designator += *it++;
			if (it != end && isalpha(*it)) designator += *it++;
			if (it != end && isalpha(*it)) designator += *it++;
			if (it != end && isalpha(*it)) designator += *it++;
			for (unsigned i = 0; i < sizeof(zones) / sizeof(Zone); ++i)
			{
				if (designator == zones[i].designator)
				{
					tzd = zones[i].tzdifferencial;
					break;
				}
			}
		}
		if (it != end && (*it == '+' || *it == '-'))
		{
			int sign = *it == '+' ? 1 : -1;
			++it;
			int hours = 0;
			PARSE_NUMBER_N(hours, 2);
			if (it != end && *it == ':') ++it;
			int minutes = 0;
			PARSE_NUMBER_N(minutes, 2);
			tzd += sign*(hours * 3600 + minutes * 60);
		}
	}

	result = tzd;
}


bool TimeParser::ParseMonth(string::const_iterator& it, const string::const_iterator& end, int& result) {

	string month;
	while (it != end && (isspace(*it) || ispunct(*it))) ++it;
	bool isFirst = true;
	while (it != end && isalpha(*it)) {

		char ch = (*it++);
		if (isFirst) { month += toupper(ch); isFirst = false; }
		else month += tolower(ch);
	}

	if (month.length() < 3)
		return false;

	for (int i = 0; i < 12; ++i) {
		if (Time::MONTH_NAMES[i].find(month) == 0) {
			result = i + 1;
			return true;
		}
	}
	return false;
}


bool TimeParser::ParseAMPM(string::const_iterator& it, const string::const_iterator& end, int& result) {

	string ampm;
	while (it != end && (isspace(*it) || ispunct(*it))) ++it;
	while (it != end && isalpha(*it)) {

		char ch = (*it++);
		ampm += toupper(ch);
	}

	if (ampm == "AM") {
		
		if (result == 12)
			result = 0;
		return true;
	}
	else if (ampm == "PM") {

		if (result < 12)
			result += 12;
		return true;
	}

	return false;
}

} // namespace Mona
