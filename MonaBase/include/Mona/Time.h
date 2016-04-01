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
#include <chrono>

namespace Mona {

class Time : public virtual Object {
public:

	/// \brief Construct a Time instance with current time value
	Time() : _time(Now()) {}

	Time(const Time& other) : _time(other._time) {}

	/// \brief Construct a Time instance with defined time value
	Time(Int64 time) : _time(time) {}

	/// \brief Copy Constructor
	virtual Time& operator=(const Time& other) { _time=other._time; return *this; }

	/// \brief Time in Microseconds since epoch
	operator Int64() const { return time(); }

	/// \brief Return true if time (in 탎ec) is elapsed since creation
	bool isElapsed(Int64 time) const {return elapsed()>time;}

	/// \brief Return time elapsed since creation (in 탎ec)
	Int64 elapsed() const { return Now() - time(); }

	/// \brief Update the time object with current time
	virtual Time& update() { _time = Now(); return *this; }

	/// \brief Update the time object with time parameter (in msec)
	virtual Time& update(Int64 time) { _time = time; return *this; }
	virtual Time& operator=(Int64 time) { _time = time; return *this; }

	/// \brief add 탎ec to the Mona time instance
	virtual Time& operator+= (Int64 time) { _time += time; return *this; }
	
	/// \brief reduce the Mona time instance (in 탎ec)
	virtual Time& operator-= (Int64 time) { _time -= time; return *this; }

	static Int64 Now() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();  }
protected:
	virtual Int64 time() const { return _time; }
private:
	Int64	_time; // time en milliseconds with as reference 1/1/1970

};

} // namespace Mona

