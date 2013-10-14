//
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

#include "Mona/Parameters.h"
#include "Poco/NumberParser.h"
#include "Poco/String.h"

using namespace std;
using namespace Poco;

namespace Mona {

bool Parameters::getNumber(const string& key, double& value) const {
	string temp;
	if (getRaw(key, temp))
		return NumberParser::tryParseFloat(temp, value);
	return false;
}
bool Parameters::getNumber(const string& key, int& value) const {
	string temp;
	if (getRaw(key, temp))
		return NumberParser::tryParse(temp, value);
	return false;
}
bool Parameters::getBool(const string& key, bool& value) const {
	string temp;
	if (!getRaw(key, temp))
		return false;
	int n;
	value = NumberParser::tryParse(temp, n) && n != 0 || (icompare(temp, "false") != 0 && icompare(temp, "no") != 0 && icompare(temp, "off") != 0);
	return true;
}

} // namespace Mona
