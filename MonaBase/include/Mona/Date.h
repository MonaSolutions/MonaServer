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

#pragma once

#include "Mona/Mona.h"
#include "Mona/Time.h"
#include "Mona/Exceptions.h"

namespace Mona {


class Date : public Time, public virtual Object {
public:
	static const char* ISO8601_FORMAT; 		/// 2005-01-01T12:00:00+01:00 | 2005-01-01T11:00:00Z
	static const char* ISO8601_FRAC_FORMAT;	/// 2005-01-01T12:00:00.000000+01:00 | 2005-01-01T11:00:00.000000Z
	static const char* ISO8601_SHORT_FORMAT; 		// 20050101T120000+01:00 | 20050101T110000Z
	static const char* ISO8601_SHORT_FRAC_FORMAT;	// 20050101T120000.000000+01:00 | 20050101T110000.000000Z
	static const char* RFC822_FORMAT;		/// Sat, 1 Jan 05 12:00:00 +0100 | Sat, 1 Jan 05 11:00:00 GMT
	static const char* RFC1123_FORMAT;		/// Sat, 1 Jan 2005 12:00:00 +0100 | Sat, 1 Jan 2005 11:00:00 GMT
	static const char* HTTP_FORMAT;			/// Sat, 01 Jan 2005 12:00:00 +0100 | Sat, 01 Jan 2005 11:00:00 GMT
	static const char* RFC850_FORMAT;		/// Saturday, 1-Jan-05 12:00:00 +0100 | Saturday, 1-Jan-05 11:00:00 GMT
	static const char* RFC1036_FORMAT;		/// Saturday, 1 Jan 05 12:00:00 +0100 | Saturday, 1 Jan 05 11:00:00 GMT
	static const char* ASCTIME_FORMAT;		/// Sat Jan  1 12:00:00 2005
	static const char* SORTABLE_FORMAT;		/// 2005-01-01 12:00:00


	enum Type {
		GMT = (Int32)0x7FFFFFFF, /// Special value for offset (Int32 minimum)
		LOCAL = (Int32)0x80000000 /// Special value for offset(Int32 maximum)
	};

	static bool  IsLeapYear(Int32 year) { return (year % 400 == 0) || (!(year & 3) && year % 100); }

	// build a NOW date, not initialized (is null)
	// /!\ Keep 'Type' to avoid confusion with "build from time" constructor, if a explicit Int32 offset is to set, use Date::setOffset or "build from time" contructor
	explicit Date(Type offset=LOCAL) : _isDST(false),_year(0), _month(0), _day(0), _weekDay(7),_hour(0), _minute(0), _second(0), _millisecond(0), _changed(false), _offset((Int32)offset),_isLocal(true) {}
	
	// build from time
	explicit Date(Int64 time,Int32 offset=LOCAL) : _isDST(false),_year(0), _month(0), _day(0),  _weekDay(7),_hour(0), _minute(0), _second(0), _millisecond(0), _changed(false), _offset(offset),_isLocal(true), Time(time) {}

	// build from other  date
	explicit Date(const Date& other) : Time((Time&)other), _isDST(other._isDST),_year(other._year), _month(other._month), _day(other._day),  _weekDay(other._weekDay),_hour(other._hour), _minute(other._minute), _second(other._second), _millisecond(other._millisecond), _changed(other._changed), _offset(other._offset),_isLocal(other._isLocal) {
	}
	
	// build from date
	explicit Date(Int32 year, UInt8 month, UInt8 day, Int32 offset=LOCAL) : _isDST(false),_year(0), _month(0), _day(0), _weekDay(7),_hour(0), _minute(0), _second(0), _millisecond(0), _changed(true), _offset(0),_isLocal(true), Time(0) {
		update(year,month,day,offset);
	}

	// build from clock
	explicit Date(UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond=0) : _isDST(false),_year(0), _month(1), _day(6), _weekDay(7),_hour(0), _minute(0), _second(0), _millisecond(0), _changed(true), _offset(0),_isLocal(false), Time(0) {
		setClock(hour,minute,second,millisecond);
	}

	// build from date+clock
	explicit Date(Int32 year, UInt8 month, UInt8 day, UInt8 hour=0, UInt8 minute=0, UInt8 second=0, UInt16 millisecond=0, Int32 offset=LOCAL) : _isDST(false),_year(0), _month(0), _day(0), _weekDay(7),_hour(0), _minute(0), _second(0), _millisecond(0), _changed(true), _offset(0),_isLocal(true), Time(0) {
		update(year,month,day,hour,minute,second,millisecond,offset);
	}

	 // now
	Date& update() { return update(Time::Now()); }
	// /!\ Keep 'Type' to avoid confusion with 'update(Int64 time)'
	Date& update(Type offset) { return update(Time::Now(),offset); }

	// from other date
	Date& update(const Date& date);

	// from time
	Date& update(Int64 time) { return update(time, _isLocal ? Int32(LOCAL) : _offset); }
	Date& update(Int64 time,Int32 offset);

	// from date
	Date& update(Int32 year, UInt8 month, UInt8 day);
	Date& update(Int32 year, UInt8 month, UInt8 day, Int32 offset) { update(year, month, day); setOffset(offset); return *this; }

	// from date+clock
	Date& update(Int32 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond = 0);
	Date& update(Int32 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, Int32 offset) { update(year, month, day, hour, minute, second,millisecond); setOffset(offset); return *this; }

	/// from string
	bool update(Exception& ex, const char* value, const char* format = NULL) { return update(ex, value, std::string::npos, format); }
	bool update(Exception& ex, const char* value, std::size_t count, const char* format = NULL);
	bool update(Exception& ex, const std::string& value, const char* format = NULL) { return update(ex, value.data(), format); }

	Date& operator=(Int64 time) { update(time); return *this; }
	Date& operator=(const Date& date) { update(date); return *this; }
	Date& operator+= (Int64 time) { update(this->time()+time); return *this; }
	Date& operator-= (Int64 time) { update(this->time()-time); return *this; }

	// to string
	std::string& toString(const char* format, std::string& value) const;

	// to time
	Int64 time() const;

	/// GETTERS
	// date
	Int32	year() const			{ if (_day == 0) init(); return _year; }
	UInt8	month() const			{ if (_day == 0) init(); return _month; }
	UInt8	day() const				{ if (_day == 0) init(); return _day; }
	UInt8	weekDay() const;
	UInt16	yearDay() const;
	// clock
	UInt32  clock() const			{ if (_day == 0) init(); return _hour*3600000L + _minute*60000L + _second*1000L + _millisecond; }
	UInt8	hour() const			{ if (_day == 0) init(); return _hour; }
	UInt8	minute() const			{ if (_day == 0) init(); return _minute; }
	UInt8	second() const			{ if (_day == 0) init(); return _second; }
	UInt16	millisecond() const		{ if (_day == 0) init(); return _millisecond; }
	// offset
	Int32	offset() const;
	bool	isGMT() const			{ if (_day == 0) init(); return offset()==0 && !_isLocal; }
	bool	isDST() const			{ offset(); /* <= allow to refresh _isDST */ return _isDST; }

	/// SETTERS
	// date
	void	setYear(Int32 year);
	void	setMonth(UInt8 month);
	void	setDay(UInt8 day);
	// clock
	void	setClock(UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond=0);
	void	setHour(UInt8 hour);
	void	setMinute(UInt8 minute);
	void	setSecond(UInt8 second);
	void	setMillisecond(UInt16 millisecond);
	// offset
	void	setOffset(Int32 offset);

private:
	void  init() const { _day = 1; ((Date*)this)->update(Time::time(), _offset); }
	void  computeWeekDay(Int64 days);
	bool  parseAuto(Exception& ex, const char* data, std::size_t count);
	void  formatTimezone(std::string& value, bool bISO = true) const;

	
	Int32			_year;
	UInt8			_month; // 1 to 12
	mutable UInt8	_day; // 1 to 31
	mutable UInt8	_weekDay; // 0 to 6 (sunday=0, monday=1) + 7 (unknown)
	UInt8			_hour; // 0 to 23
	UInt8			_minute;  // 0 to 59
	UInt8			_second;	// 0 to 59
	UInt16			_millisecond; // 0 to 999
	mutable Int32	_offset; // gmt offset
	mutable bool	_isDST; // means that the offset is a Daylight Saving Time offset
	mutable bool	_isLocal; // just used when offset is on the special Local value!

	mutable bool _changed; // indicate that date information has changed, we have to refresh time value
};


} // namespace Mona

