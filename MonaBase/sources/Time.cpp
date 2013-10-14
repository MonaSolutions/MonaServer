/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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

#include "Mona/Time.h"
#include "Mona/TimeParser.h"
#include "Mona/String.h"

using namespace std;

namespace Mona {

const std::string Time::ISO8601_FORMAT("%Y-%m-%dT%H:%M:%S%z");
const std::string Time::ISO8601_FRAC_FORMAT("%Y-%m-%dT%H:%M:%s%z");
const std::string Time::RFC822_FORMAT("%w, %e %b %y %H:%M:%S %Z");
const std::string Time::RFC1123_FORMAT("%w, %e %b %Y %H:%M:%S %Z");
const std::string Time::HTTP_FORMAT("%w, %d %b %Y %H:%M:%S %Z");
const std::string Time::RFC850_FORMAT("%W, %e-%b-%y %H:%M:%S %Z");
const std::string Time::RFC1036_FORMAT("%W, %e %b %y %H:%M:%S %Z");
const std::string Time::ASCTIME_FORMAT("%w %b %f %H:%M:%S %Y");
const std::string Time::SORTABLE_FORMAT("%Y-%m-%d %H:%M:%S");

const string Time::WEEKDAY_NAMES[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};


const string Time::MONTH_NAMES[] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

Time::Time(struct tm& tmtime, int milli /* = 0 */, int micro /* = 0 */) {
	
	// Convert Local to GMT
	time_t gmttime = timegm(&tmtime);

	Int64 monatime = (gmttime * 1000000) + (milli * 1000) + (micro);
	
	_time = chrono::system_clock::time_point(chrono::microseconds(monatime));
}

bool Time::isValid(struct tm& tmtime, int milli /* = 0 */, int micro /* = 0 */) {

	// Does it can be converted?
	if (timegm(&tmtime) == -1)
		return false;	
	
	if (milli < 0 || milli > 999)
		return false;

	if (micro < 0 || micro > 999)
		return false;
	
	return true;
}

bool Time::isElapsed(Int64 time) const {

	chrono::microseconds dt = chrono::microseconds(time);
	chrono::time_point<chrono::system_clock> tp = chrono::system_clock::now();

	// Get the time elapsed
	chrono::microseconds dtSinceNow = chrono::duration_cast<chrono::microseconds>(tp - _time);

	// Is the time elapsed?
	return dtSinceNow > dt;
}

Int64 Time::elapsed() const {
	chrono::time_point<chrono::system_clock> tp = chrono::system_clock::now();

	// Get the time elapsed
	chrono::microseconds dtSinceNow = chrono::duration_cast<chrono::microseconds>(tp - _time);

	// Is the time elapsed?
	return dtSinceNow.count();
}

bool Time::toString(std::string &out, const string& fmt, int timezone /*= UTC*/) const {

	out.reserve(32);

	time_t date = chrono::system_clock::to_time_t(_time);
	struct tm * datetm = gmtime(&date);

	std::string::const_iterator it = fmt.begin();
	std::string::const_iterator end = fmt.end();
	while (it != end)
	{
		if (*it == '%')
		{
			if (++it != end)
			{
				switch (*it)
				{
				case 'w': out.append(WEEKDAY_NAMES[datetm->tm_wday], 0, 3); break;
				case 'W': out.append(WEEKDAY_NAMES[datetm->tm_wday]); break;
				case 'b': out.append(MONTH_NAMES[datetm->tm_mon], 0, 3); break;
				case 'B': out.append(MONTH_NAMES[datetm->tm_mon]); break;
				case 'd': String::Format(out, pair<const char *, int>( "%02d", datetm->tm_mday)); break;
				case 'e': String::Format(out, datetm->tm_mday); break;
				case 'f': String::Format(out, pair<const char *, int>("%2d", datetm->tm_mday)); break;
				case 'm': String::Format(out, pair<const char *, int>("%02d", datetm->tm_mon + 1)); break;
				case 'n': String::Format(out, datetm->tm_mon + 1); break;
				case 'o': String::Format(out, pair<const char *, int>("%2d", datetm->tm_mon + 1)); break;
				case 'y': String::Format(out, pair<const char *, int>("%02d", datetm->tm_year % 100)); break;
				case 'Y': String::Format(out, pair<const char *, int>("%04d", datetm->tm_year + 1900)); break;
				case 'H': String::Format(out, pair<const char *, int>("%02d", datetm->tm_hour)); break;
				case 'h': String::Format(out, pair<const char *, int>("%02d", hour2AMPM(datetm->tm_hour) )); break;
				case 'a': String::Format(out, (datetm->tm_hour < 12) ? "am" : "pm"); break;
				case 'A': String::Format(out, (datetm->tm_hour < 12) ? "AM" : "PM"); break;
				case 'M': String::Format(out, pair<const char *, int>("%02d", datetm->tm_min)); break;
				case 'S': String::Format(out, pair<const char *, int>("%02d", datetm->tm_sec)); break;
				case 's': String::Format(out, pair<const char *, int>("%02d", datetm->tm_sec));
					out += '.';
					String::Format(out, pair<const char *, int>("%06d", millisec()*1000 + microsec()));
					break;
				case 'i': String::Format(out, pair<const char *, int>("%03d", millisec() )); break;
				case 'c': String::Format(out, millisec() / 100); break;
				case 'F': String::Format(out, pair<const char *, int>("%06d", millisec() * 1000 + microsec())); break;
				case 'z': tzFormat(out, timezone); break;
				case 'Z': tzFormat(out, timezone, false); break;
				default:  out += *it;
				}
				++it;
			}
		}
		else out += *it++;
	}

	return true;
}

void Time::tzFormat(std::string& str, int tzDifferential, bool bISO /* = true */) const {

	if (tzDifferential != UTC) {

		int tzd = (tzDifferential < 0)? -tzDifferential : tzDifferential;
		str += (tzDifferential < 0) ? '-' : '+';
		String::Format(str, pair<const char *, int>("%02d", tzd / 3600));
		if (bISO) str += ':';
		String::Format(str, pair<const char *, int>("%02d", (tzd % 3600) / 60));
	}
	else {

		str += (bISO)? "Z" : "GMT";
	}
}

bool Time::fromString(const std::string &in) {

	int tz = 0;

	return TimeParser::tryParse(in, *this, tz);
}

bool Time::toLocal(struct tm& tmtime) const {

	time_t date = chrono::system_clock::to_time_t(_time);
	struct tm * tmp = localtime(&date);
	tmtime = *tmp;

	return tmp != NULL;
}

bool Time::toGMT(struct tm& tmtime) const {

	time_t date = chrono::system_clock::to_time_t(_time);
	struct tm * tmp = gmtime(&date);
	tmtime = *tmp;

	return tmp != NULL;
}

} // namespace Mona
