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

#pragma once

#include "Mona/Mona.h"
#include <chrono>
#include <list>

namespace Mona {

class Time : ObjectFix {
public:

	// names used by formatter and parser
	static const std::string WEEKDAY_NAMES[7];
	static const std::string MONTH_NAMES[12];

	static const std::string ISO8601_FORMAT;		///   2005-01-01T12:00:00+01:00 | 2005-01-01T11:00:00Z
	static const std::string ISO8601_FRAC_FORMAT;	///   2005-01-01T12:00:00.000000+01:00 | 2005-01-01T11:00:00.000000Z
	static const std::string RFC822_FORMAT;			/// Sat, 1 Jan 05 12:00:00 +0100 | Sat, 1 Jan 05 11:00:00 GMT
	static const std::string RFC1123_FORMAT;		/// Sat, 1 Jan 2005 12:00:00 +0100 | Sat, 1 Jan 2005 11:00:00 GMT
	static const std::string HTTP_FORMAT;			///   Sat, 01 Jan 2005 12:00:00 +0100 | Sat, 01 Jan 2005 11:00:00 GMT
	static const std::string RFC850_FORMAT;			/// Saturday, 1-Jan-05 12:00:00 +0100 | Saturday, 1-Jan-05 11:00:00 GMT
	static const std::string RFC1036_FORMAT;		/// Saturday, 1 Jan 05 12:00:00 +0100 | Saturday, 1 Jan 05 11:00:00 GMT
	static const std::string ASCTIME_FORMAT;		/// Sat Jan  1 12:00:00 2005
	static const std::string SORTABLE_FORMAT;		/// 2005-01-01 12:00:00

	enum {
		UTC = 0xFFFF /// Special value for timeZoneDifferential 
	};

	/// \brief Construct a Time instance with current time value
	Time() : _time(std::chrono::system_clock::now()) {}

	/// \brief Construct a Time instance with defined time value
	Time(Int64 time) : _time(std::chrono::microseconds(time)) {}

	/// \brief Construct a Time instance with a time_point value
	Time(const std::chrono::system_clock::time_point& tp) : _time(tp) {}

	/// \brief Construct a Time instance with a struct tm (GMT)
	/// WARN : the tmtime parameter could be modified in case of
	/// invalid date
	Time(struct tm& tmtime, int milli = 0, int micro = 0) { update(tmtime, milli, micro); }

	/// \brief Construct the object by copying time's value
	Time(const Time& time) : _time(time._time) {}

	/// \brief Destructor
	~Time() { }

	/// \brief Update the time object with current time
	void update() { _time = std::chrono::system_clock::now(); }

	/// \brief Update the time object with time parameter (in msec)
	void update(Int64 time) { _time = std::chrono::system_clock::time_point(std::chrono::microseconds(time)); }

	/// \brief Update the time object with the time_point value
	void update(const std::chrono::system_clock::time_point& tp) { _time = tp; }

	/// \brief Update the time object with struct tm parameter
	/// WARN : the tmtime parameter could be modified in case of
	/// invalid date
	void update(struct tm& tmtime, int milli = 0, int micro = 0);

	/// \brief Update the object by copying time's value
	void update(const Time& time) { _time = time._time; }

	/// \brief time_point object accessor
	std::chrono::system_clock::time_point getTimePoint() const { return _time; };

	/// \brief Time in Microseconds since epoch
	Int64 toInt() const { return std::chrono::duration_cast<std::chrono::microseconds>(_time.time_since_epoch()).count(); }

	/// \brief Return milliseconds of the time object
	Int16 millisec() const { return ((toInt() / 1000) % 1000); }

	/// \brief Return microseconds of the time object
	Int16 microsec() const { return (toInt() % 1000); }

	/// \brief add 탎ec to the Mona time instance
	Time& operator+= (Int64 time) { _time += std::chrono::microseconds(time); return *this; }
	
	/// \brief reduce the Mona time instance (in 탎ec)
	Time& operator-= (Int64 time) { _time -= std::chrono::microseconds(time); return *this; }

	/// \brief Return true if time (in 탎ec) is elapsed since creation
	bool isElapsed(Int64 time) const;

	/// \brief Return time elapsed since creation (in 탎ec)
	Int64 elapsed() const;

	/// \brief Convert to a string with a particular format
	bool toString(std::string& out, const std::string& fmt, int timezone = UTC) const;
	bool toLocaleString(std::string& out, const std::string& fmt, int timezone = UTC) const;

	/// \brief Try to parse a date string and assign the Time instance
	bool fromString(const std::string &in);

	/// \brief Is this time valid?
	/// WARN : the tmtime parameter could be modified in case of invalid date
	/// Valid params are :
	/// - A tmtime between the 1/1/1970 and the 1/1/3001
	/// - 0 <= milli < 1000
	/// - 0 <= micro < 1000
	static bool isValid(struct tm& tmtime, int milli = 0, int micro = 0);

	/// \brief Convert Mona Time to local time
	bool toLocal(struct tm& tmtime) const;

	/// \brief Convert Mona Time to time GMT
	bool toGMT(struct tm& tmtime) const;

private:
	std::chrono::system_clock::time_point _time;

	void formatDate(std::string& out, const struct tm& datetm, const std::string& fmt, int timezone = UTC) const;
	void tzFormat(std::string& str, int tzDifferential, bool bISO = true) const;

	int hour2AMPM(int hour) const;
};

inline int Time::hour2AMPM(int hour) const {

	if (hour < 1)
		return 12;
	else if (hour > 12)
		return hour - 12;
	
	return hour;
}

// Operators definition for class Time
inline bool operator== (const Time& lhs, const Time& rhs) { return lhs.getTimePoint() == rhs.getTimePoint(); }

inline bool operator!= (const Time& lhs, const Time& rhs) { return lhs.getTimePoint() != rhs.getTimePoint(); }

inline bool operator<  (const Time& lhs, const Time& rhs) { return lhs.getTimePoint() < rhs.getTimePoint(); }

inline bool operator>  (const Time& lhs, const Time& rhs) { return lhs.getTimePoint() > rhs.getTimePoint(); }

inline bool operator>= (const Time& lhs, const Time& rhs) { return lhs.getTimePoint() >= rhs.getTimePoint(); }

inline bool operator<= (const Time& lhs, const Time& rhs) { return lhs.getTimePoint() <= rhs.getTimePoint(); }

inline Time	operator+ (const Time& lhs, const Int64 dtn) { return Time(lhs.getTimePoint() + std::chrono::microseconds(dtn)); }

inline Time	operator+ (const Int64 dtn, const Time& rhs) { return Time(std::chrono::microseconds(dtn) + rhs.getTimePoint()); }

inline Time	operator- (const Time& lhs, const Int64 dtn) { return Time(lhs.getTimePoint() - std::chrono::microseconds(dtn)); }

inline Int64 operator- (const Time& lhs, const Time& rhs) { return std::chrono::duration_cast<std::chrono::microseconds>(lhs.getTimePoint() - rhs.getTimePoint()).count(); }

} // namespace Mona

