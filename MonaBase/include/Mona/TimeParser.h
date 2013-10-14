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

class TimeParser {
public:

	static bool tryParse(const std::string& in, Time& dateTime, int& tz);
	static bool parse(const std::string& fmt, const std::string& in, Time& time, int& tzdifferencial);

	static bool isDigit(char ch);
	static bool isAlpha(char ch);
	static bool isSpace(char ch);
	static bool isPunct(char ch);
	static bool isLower(int ch);
	static bool isUpper(int ch);
	static int toLower(int ch);
	static int toUpper(int ch);
	
private:

	static bool parseTZD(std::string::const_iterator& it, const std::string::const_iterator& end, int& result);
	static bool parseMonth(std::string::const_iterator& it, const std::string::const_iterator& end, int& result);
	static bool parseAMPM(std::string::const_iterator& it, const std::string::const_iterator& end, int& result);
};

inline bool TimeParser::isDigit(char ch) { return (ch >= '0') && (ch <= '9'); }

inline bool TimeParser::isAlpha(char ch) { return ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')); }

inline bool TimeParser::isSpace(char ch) { return (ch == ' ') || ((ch >= '\t') && (ch <= '\r')); }

inline bool TimeParser::isPunct(char ch) { return ((ch >= '!') && (ch <= '/')) || ((ch >= ':') && (ch <= '@')) || ((ch >= '[') && (ch <= '`')) || ((ch >= '{') && (ch <= '~')); }

inline bool TimeParser::isLower(int ch) { return (ch >= 'a') && (ch <= 'z'); }

inline bool TimeParser::isUpper(int ch) { return (ch >= 'A') && (ch <= 'Z'); }

inline int TimeParser::toLower(int ch) { return (isUpper(ch)) ? ch + 32 : ch; }

inline int TimeParser::toUpper(int ch) { return (isLower(ch)) ? ch - 32 : ch; }

} // namespace Mona

