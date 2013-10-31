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
#include <list>

namespace Mona {

class Time;

class TimeParser : virtual Static {
public:

	static bool TryParse(const std::string& in, Time& dateTime, int& tz);
	static bool Parse(const std::string& fmt, const std::string& in, Time& time, int& tzdifferencial);
	
private:

	static void ParseTZD(std::string::const_iterator& it, const std::string::const_iterator& end, int& result);
	static bool ParseMonth(std::string::const_iterator& it, const std::string::const_iterator& end, int& result);
	static bool ParseAMPM(std::string::const_iterator& it, const std::string::const_iterator& end, int& result);
};

} // namespace Mona

