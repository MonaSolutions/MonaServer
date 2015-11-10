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

#include "Mona/Date.h"
#include "Mona/Timezone.h"
#include "math.h"


using namespace std;

namespace Mona {


const char* Date::ISO8601_FORMAT("%Y[-%m-%dT%H:%M:%S%z]");  	  // 2005-01-01T12:00:00+01:00 | 2005-01-01T11:00:00Z
const char* Date::ISO8601_FRAC_FORMAT("%Y[-%m-%dT%H:%M:%s%z]");   // 2005-01-01T12:00:00.000000+01:00 | 2005-01-01T11:00:00.000000Z
const char* Date::ISO8601_SHORT_FORMAT("%Y[%m%dT%H%M%S%z]");  	  // 20050101T120000+01:00 | 20050101T110000Z
const char* Date::ISO8601_SHORT_FRAC_FORMAT("%Y[%m%dT%H%M%s%z]"); // 20050101T120000.000000+01:00 | 20050101T110000.000000Z
const char* Date::RFC822_FORMAT("[%w, ]%e %b %y %H:%M[:%S] %Z");  // Sat, 1 Jan 05 12:00:00 +0100 | Sat, 1 Jan 05 11:00:00 GMT
const char* Date::RFC1123_FORMAT("%w, %e %b %Y %H:%M:%S %Z");	  // Sat, 1 Jan 2005 12:00:00 +0100 | Sat, 1 Jan 2005 11:00:00 GMT
const char* Date::HTTP_FORMAT("%w, %d %b %Y %H:%M:%S %Z");		  // Sat, 01 Jan 2005 12:00:00 +0100 | Sat, 01 Jan 2005 11:00:00 GMT
const char* Date::RFC850_FORMAT("%W, %e-%b-%y %H:%M:%S %Z");	  // Saturday, 1-Jan-05 12:00:00 +0100 | Saturday, 1-Jan-05 11:00:00 GMT
const char* Date::RFC1036_FORMAT("%W, %e %b %y %H:%M:%S %Z");	  // Saturday, 1 Jan 05 12:00:00 +0100 | Saturday, 1 Jan 05 11:00:00 GMT
const char* Date::ASCTIME_FORMAT("%w %b %f %H:%M:%S %Y");		  // Sat Jan  1 12:00:00 2005
const char* Date::SORTABLE_FORMAT("%Y-%m-%d[ %H:%M:%S]");		  // 2005-01-01 12:00:00



static const char* WEEKDAY_NAMES[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

static const string MONTH_NAMES[] = {
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

static const UInt16 MonthDays[][12] = {
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
	{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335} // leap
};

static Int32 LeapYears(Int32 year) {
	Int32 result(year/4); // every 4 year
	result -= year/100; // but not divisible by 100
	result += year/400; // except divisible by 400
	result -= 477; // 1970 reference
	if (year <= 0) // cause 0 is not leap, and it's delayed in the negative: -1, -5, etc..
		--result;
	return result;
}

Date& Date::update(Int64 time,Int32 offset) {
	Time::update(time);
	if (_day == 0) {
		_offset = offset;
		return *this;
	}

	_changed = false;

	// update time with offset
	Timezone::TimeType timeType(Timezone::STANDARD);
	_offset = offset;
	_isLocal = _isDST = false;
	if (_offset == GMT)
		_offset = 0;
	else if (_offset == LOCAL) {
		_isLocal = true;
		_offset = Timezone::LocalOffset(time,timeType);
		if (timeType == Timezone::DST)
			_isDST = true;
	}
	time += _offset;

	Int64 days((Int64)floor(time / 86400000.0));

	// in first, compute weak day
	computeWeekDay(days);

	if (time >= 0)
		++days;
	Int32 year = _year = 0;
	Int32 delta(0);
	while ((year = (Int32)(days / 366)) != _year) {
		days -= delta;
		delta = year-LeapYears(year + 1970);
		days += delta;
		_year=year;
	}
	days %= 366;
	year = _year += 1970;
	bool isLeap(IsLeapYear(_year));
	if (time < 0) {
		if (_year>0 && isLeap)
			++days;
		if (days < 0)
			--_year;
		++days;
	}
	if (_year!=year)
		isLeap = IsLeapYear(_year);

	if (time >= 0) {
		if (isLeap)
			++days;
	} else if (days <= 0)
		days += (isLeap ? 366 : 365);

	_month = 1;
	UInt16 count(0);
	while (_month<12 && days > (delta=MonthDays[isLeap ? 1 : 0][_month])) {
		++_month;
		count = delta;
	}
	days -= count;

	_day = (UInt8)days;
	
	time = time%86400000;
	if (time < 0)
		time += 86400000;

	
	// just clock missing now, we can add daylight saving time if required
	if (timeType == Timezone::CHECK_DST_WITH_RULES) {
		days = Timezone::LocalOffsetUsingRules(*this,(UInt32)time,_isDST);
		time -= _offset;
		time += (_offset = (Int32)days);
	}

	_hour = (UInt8)(time / 3600000);
	time %= 3600000;
	_minute = (UInt8)(time / 60000);
	time %= 60000;
	_second = (UInt8)(time / 1000);
	time %= 1000;

	_millisecond = (UInt16)time;
	return *this;
}

Int64 Date::time() const {
	if (!_changed) // if _day==0 then _changed==false!
		return Time::time();

	Int64 time = _day - 1 + LeapYears(_year);
	bool isLeap(IsLeapYear(_year));
	if (isLeap && _year > 0)
		--time;
	 time += MonthDays[isLeap?1:0][_month-1];

	 time += (_year - 1970)*365;
	 UInt32 clock(this->clock());
	 time = time * 86400000 + clock;

	((Date&)*this).Time::update(time);
	_changed = false;
	_weekDay = 7;
	if (_offset == GMT) {
		_isLocal = _isDST = false;
		_offset = 0;
	} else if (_offset == LOCAL || _isLocal) {
		_offset = GMT;
		_offset = Timezone::LocalOffset(*this,clock, _isDST); // will call time(), offset() and weekDay()
		_isLocal = true;
	} else
		_isLocal = false;
	((Date&)*this).Time::update(time-_offset);
	return Time::time();
}

Int32 Date::offset() const {
	if (_day == 0) {
		init();
		return _offset;
	}

	if (_offset == GMT) {
		_offset = 0;
		_isDST = _isLocal = false;
		return 0;
	}
	if (_offset == LOCAL)
		_isLocal = true;
	if (!_isLocal)
		return _offset;  // _offset is a fix value

	if (_changed)
		time();  // assign _isLocal, _offset and _isDST
	else if (_offset == LOCAL) {
		_offset = GMT;
		_offset = Timezone::LocalOffset(*this, _isDST);
	}
	
	return _offset;
}

void Date::setOffset(Int32 offset) {
	if (_day == 0) {
		_offset = offset;
		return;
	}

	if (offset == LOCAL) {
		if (_offset == LOCAL || _isLocal)
			return;
		_isLocal = true;
	} else if (offset == GMT) {
		_isDST = _isLocal = false;
		if (_offset==GMT || _offset==0)
			return;
	}
	if (!_changed) // means that _offset is an integer
		Time::update(Time::time()+_offset-offset); // time + old - new
	_offset = offset;
}



Date& Date::update(const Date& date) {
	_year = date._year;
	_month = date._month;
	_day = date._day;
	_weekDay = date._weekDay;
	_hour = date._hour;
	_minute = date._minute;
	_second = date._second;
	_millisecond = date._millisecond;
	_offset = date._offset;
	_isLocal = date._isLocal;
	_isDST = date._isDST;
	Time::update((Time&)date);
	return *this;
}

Date& Date::update(Int32 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond) {
	setYear(year);
	setMonth(month);
	setClock(hour, minute, second, millisecond);
	// keep the following line in last position
	setDay(day);
	return *this;
}


Date& Date::update(Int32 year, UInt8 month, UInt8 day) {
	setYear(year);
	setMonth(month);
	// keep the following line in last position
	setDay(day);
	return *this;
}


void Date::setClock(UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond) {
	setHour(hour);
	setMinute(minute);
	setSecond(second);
	setMillisecond(millisecond);
}

void Date::setYear(Int32 year) {
	if (_day == 0)
		init();
	if (year == _year)
		return;
	_changed = true;
	_year = year;
}

void Date::setMonth(UInt8 month) {
	if (month<1)
		month = 1;
	else if (month>12)
		month = 12;
	if (_day == 0)
		init();
	if (month == _month)
		return;
	_changed = true;
	_month = month;
}

void Date::setDay(UInt8 day) {
	if (_day == 0)
		init();
	if (day == _day)
		return;

	if (day<1)
		day = 1;
	else if (day>31)
		day = 31;
	if (day > 28) {
		if (_month < 8) {
			if (_month == 2) {
				if (day >= 30)
					day = 29;
				if (day == 29 && !IsLeapYear(_year))
					day = 28;
			} else if (day==31 && !(_month & 0x01))
				day = 30;
		} else if(day==31) {
			if (_month & 0x01)
				day = 30;
		}
	}
	
	if (day == _day)
		return;
	_changed = true;
	_day = day;
}

void Date::setHour(UInt8 hour) {
	if (_day == 0)
		init();
	if (hour > 59)
		hour = 59;
	if (hour == _hour)
		return;
	_changed = true;
	_hour = hour;
}

void Date::setMinute(UInt8 minute) {
	if (_day == 0)
		init();
	if (minute>59)
		minute = 59;
	if (minute == _minute)
		return;
	_changed = true;
	_minute = minute;
}

void Date::setSecond(UInt8 second) {
	if (_day == 0)
		init();
	if (second>59)
		second = 59;
	if (second == _second)
		return;
	_changed = true;
	_second = second;
}

void Date::setMillisecond(UInt16 millisecond) {
	if (_day == 0)
		init();
	if (millisecond > 999)
		millisecond = 999;
	if (millisecond == _millisecond)
		return;
	if (!_changed)
		Time::update(time()-_millisecond+millisecond);
	_millisecond = millisecond;
}

UInt8 Date::weekDay() const {
	if (_day == 0)
		init(); // will assign _weekDay
	else if (_changed || _weekDay==7)
		((Date&)*this).computeWeekDay((Int64)floor((time()+offset()) / 86400000.0));
	return _weekDay;
}

UInt16 Date::yearDay() const {
	// 0 to 365
	if (_day == 0)
		init();
	return _day+MonthDays[IsLeapYear(_year) ? 1 : 0][_month]-1;
}

void Date::computeWeekDay(Int64 days) {
	Int64 result(days += 4);
	if (days < 0)
		++result;
	result %= 7;
	if (days<0)
		result += 6;
	_weekDay = (UInt8)result;
}


///////////// FORMATER //////////////////////////


string& Date::toString(const char* format, string& value) const {
	if (!format)
		return value;

	value.clear();

	if (_day == 0)
		init();

	UInt32 formatSize = strlen(format);
	UInt32 iFormat(0);

	while (iFormat < formatSize) {

		char c(format[iFormat++]);

		if (c != '%') {
			if (c != '[' && c != ']')
				value += c;
			continue;
		}

		if (iFormat == formatSize)
			break;
		
		switch (c = format[iFormat++]) {
			case 'w': value.append(WEEKDAY_NAMES[weekDay()], 0, 3); break;
			case 'W': value.append(WEEKDAY_NAMES[weekDay()]); break;
			case 'b': value.append(MONTH_NAMES[_month-1], 0, 3); break;
			case 'B': value.append(MONTH_NAMES[_month-1]); break;
			case 'd': String::Append(value, Format<UInt8>("%02d", _day)); break;
			case 'e': String::Append(value, _day); break;
			case 'f': String::Append(value, Format<UInt8>("%2d", _day)); break;
			case 'm': String::Append(value, Format<UInt8>("%02d", _month)); break;
			case 'n': String::Append(value, _month); break;
			case 'o': String::Append(value, Format<UInt8>("%2d", _month)); break;
			case 'y': String::Append(value, Format<Int32>("%02d", _year % 100)); break;
			case 'Y': String::Append(value, Format<Int32>("%04d", _year)); break;
			case 'H': String::Append(value, Format<UInt8>("%02d", _hour)); break;
			case 'h': String::Append(value, Format<UInt8>("%02d",(_hour<1 ? 12 : (_hour>12 ? (_hour-12) : _hour)))); break;
			case 'a': String::Append(value, (_hour < 12) ? "am" : "pm"); break;
			case 'A': String::Append(value, (_hour < 12) ? "AM" : "PM"); break;
			case 'M': String::Append(value, Format<UInt8>("%02d", _minute)); break;
			case 'S': String::Append(value, Format<UInt8>("%02d", _second)); break;
			case 's': String::Append(value, Format<UInt8>("%02d", _second));
				value += '.';
			case 'F':
			case 'i': String::Append(value, Format<UInt16>("%03d", _millisecond)); break;
			case 'c': String::Append(value, _millisecond / 100); break;
			case 'z': formatTimezone(value); break;
			case 'Z': formatTimezone(value, false); break;
			default: value += c;
		}
	}
	return value;
}


void Date::formatTimezone(string& value, bool bISO) const {
	if (isGMT()) {
		value += (bISO) ? "Z" : "GMT";
		return;
	}
	value += (_offset < 0) ? '-' : '+';
	UInt32 offset = abs(_offset);
	String::Append(value, Format<UInt32>("%02d", offset / 3600000));
	if (bISO)
		value += ':';
	String::Append(value, Format<UInt32>("%02d", (offset % 3600000) / 60000));
}



///////////// PARSER //////////////////////////



#define CAN_READ (*current && size!=0)
#define READ	 (--size,*current++)

#define SKIP_DIGITS \
	while (CAN_READ && isdigit(*current)) READ;

#define PARSE_NUMBER(var) \
	while (CAN_READ && isdigit(*current)) var = var * 10 + (READ - '0')

#define PARSE_NUMBER_N(var, n) \
	{ size_t i = 0; while (i++ < n && CAN_READ && isdigit(*current)) var = var * 10 + (READ - '0');}

#define PARSE_FRACTIONAL_N(var, n) \
	{ size_t i = 0; while (i < n && CAN_READ && isdigit(*current)) { var = var * 10 + (READ - '0'); i++; } while (i++ < n) var *= 10; }





bool Date::update(Exception& ex, const char* current, size_t size, const char* format) {
	if (!format)
		return parseAuto(ex, current,size);

	UInt8 month(0), day(0), hour(0), minute(0), second(0);
	Int32 year(0), offset(LOCAL);
	UInt16 millisecond(0);
	int microsecond(0);
	bool isDST(false);
	int optional(0);

	while (*format) {

		char c(*(format++));

		if (c == '[') {
			if (optional>=0)
				++optional;
			else
				--optional;
			continue;
		} else if (c == ']') {
			if (optional > 0)
				--optional;
			else
				++optional;
			continue;
		} else if (c != '%') {
			if (c == '?') {} 
			else if (optional<0)
				continue;
			else if (optional > 0) {
				if (!CAN_READ || c != *current) {
					optional = -optional;
					continue;
				}
			} else if (!CAN_READ || c != *current)
				return false;
			READ;
			continue;
		}
			
		if (!(*format))
			break;

		switch (*format) {
			default:
				if (optional == 0)
					ex.set(Exception::FORMATTING, "Unknown date ",*format," pattern");
				break;
			case '%': // to catch %% case
				if (c != '%')
					return false;
				break;
			case 'w':
			case 'W':
				while (CAN_READ && isalpha(*current))
					READ;
				break;
			case 'b':
			case 'B': {
				month = 0;
				string value;
				bool isFirst = true;
				while (CAN_READ && isalpha(*current)) {
					char ch(READ);
					if (isFirst) {
						value += toupper(ch);
						isFirst = false;
					} else
						value += tolower(ch);
				}

				if (value.length() >= 3) {
					for (int i = 0; i < 12; ++i) {
						if (MONTH_NAMES[i].find(value) == 0) {
							month = i + 1;
							break;
						}
					}
				}
				if (month == 0 && optional == 0)
					ex.set(Exception::FORMATTING, "Impossible to parse ", value, " as a valid month");
				break;
			}
			case 'd':
			case 'e':
			case 'f':
				if (CAN_READ && isspace(*current))
					READ;
				PARSE_NUMBER_N(day, 2);
				break;
			case 'm':
			case 'n':
			case 'o':
				PARSE_NUMBER_N(month, 2);
				break;
			case 'y':
				PARSE_NUMBER_N(year, 2);
				if (year >= 70)
					year += 1900;
				else
					year += 2000;
				break;
			case 'Y':
				PARSE_NUMBER_N(year, 4);
				break;
			case '_':
				PARSE_NUMBER(year);
				if (year < 100) {
					if (year >= 70)
						year += 1900;
					else
						year += 2000;
				}
				break;
			case 'H':
			case 'h':
				PARSE_NUMBER_N(hour, 2);
				break;
			case 'a':
			case 'A': {
				const char* ampm(current);
				UInt32 count(0);
				while (CAN_READ && isalpha(*current)) {
					READ;
					++count;
				}
				if (String::ICompare(ampm,"AM",count)==0) {
					if (hour == 12)
						hour = 0;
				} else if (String::ICompare(ampm,"PM",count)==0) {
					if (hour < 12)
						hour += 12;
				} else if (optional==0)
					ex.set(Exception::FORMATTING, "Impossible to parse ", ampm, " as a valid AM/PM information");
				break;
			}
			case 'M':
				PARSE_NUMBER_N(minute, 2);
				break;
			case 'S':
				PARSE_NUMBER_N(second, 2);
				break;
			case 's':
				PARSE_NUMBER_N(second, 2);
				if (CAN_READ && (*current == '.' || *current == ',')) {
					READ;
					PARSE_FRACTIONAL_N(millisecond, 3);
					PARSE_FRACTIONAL_N(microsecond, 3);
					SKIP_DIGITS;
				}
				break;
			case 'i':
				PARSE_NUMBER_N(millisecond, 3);
				break;
			case 'c':
				PARSE_NUMBER_N(millisecond, 1);
				millisecond *= 100;
				break;
			case 'F':
				PARSE_FRACTIONAL_N(millisecond, 3);
				PARSE_FRACTIONAL_N(microsecond, 3);
				SKIP_DIGITS;
				break;
			case 'z':
			case 'Z':

				offset = LOCAL;
				const char* code(current);
				UInt32 count(0);
				while (CAN_READ && isalpha(*current)) {
					READ;
					++count;
				}
				if (count>0) {
					if (*current)
						offset = Timezone::Offset(string(code,count),isDST);
					else
						offset = Timezone::Offset(code,isDST);
				}
				if (CAN_READ && (*current == '+' || *current == '-')) {
					if (offset==GMT || offset==LOCAL)
						offset = 0;

					int sign = READ == '+' ? 1 : -1;
					int hours = 0;
					PARSE_NUMBER_N(hours, 2);
					if (CAN_READ && *current == ':')
						READ;
					int minutes = 0;
					PARSE_NUMBER_N(minutes, 2);
					offset += sign*(hours * 3600 + minutes * 60)*1000;
				}			
				break;
		}
		++format;
	}

	update(year,month,day,hour,minute,second,millisecond,offset);
	_isDST = isDST;

	if (microsecond > 0)
		ex.set(Exception::FORMATTING, "microseconds information lost, not supported by Mona Date system");
	return true;
}

bool Date::parseAuto(Exception& ex, const char* data, size_t count) {

	size_t length(0),tPos(0);
	bool digit(false);
	bool digits(false);
	const char* current = data;
	size_t size(count);

	while(CAN_READ && length<50) {
		char c(*current);
		if (digit && c == 'T')
			tPos = length;
		if (length<10) {
			if (length == 0)
				digit = isdigit(c) != 0;
			else if (length <= 2) {
				if (length == 1) { 
					if (digit) {
						digits = true;
						if (!isdigit(c))
							return update(ex, data, count, "%e?%b?%_ %H:%M[:%S %Z]");
					}
				} else if (digits && !isdigit(c))
					return update(ex, data, count, "%e?%b?%_ %H:%M[:%S %Z]");
			} else if (length==3 && c==' ')
				return update(ex,data,count, ASCTIME_FORMAT);

			if (c == ',') {
				if (length == 3)
					return update(ex,data, count, "%w, %e?%b?%_ %H:%M[:%S %Z]");
				return update(ex,data, count, "%W, %e?%b?%_ %H:%M[:%S %Z]");
			}

			++length;
			READ;
			continue;
		}
		
		// length >= 10
		
		if (length == 10) {
			if(!digit)
				return false;
			if (c == ' ')
				return update(ex, data, count, SORTABLE_FORMAT);
			if (!tPos)
				return false;
		}
	
		if (c == '.' || c == ',') {
			if (tPos==8)
				return update(ex, data, count, "%Y%m%dT%H%M%s[%z]");
			return update(ex, data, count,"%Y-%m-%dT%H:%M:%s[%z]"); //  ISO8601_FRAC_FORMAT
		}

		READ;
		++length;
	}

	if (!digit)
		return false;
	if (length == 10)
		return update(ex, data, count, SORTABLE_FORMAT);
	if (tPos==10)
		return update(ex, data, count, "%Y-%m-%dT%H:%M:%S[%z]"); //  ISO8601_FORMAT
	if (tPos==8) // compact format (20050108T123000, 20050108T123000Z, 20050108T123000.123+0200)
		return update(ex, data, count, "%Y%m%dT%H%M%s[%z]");
	return false;
}


} // namespace Mona
