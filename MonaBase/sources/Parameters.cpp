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
#include "Mona/String.h"
#include "Mona/Exceptions.h"

using namespace std;

namespace Mona {

bool Parameters::getNumber(const string& key, double& value) const {
	string temp;
	if (getRaw(key, temp)) {
		Exception ex;
		value = String::ToNumber<double>(ex, temp);
		if (!ex)
			return true;
	}
	return false;
}

bool Parameters::getNumber(const string& key, int& value) const {
	string temp;
	if (getRaw(key, temp)) {
		Exception ex;
		value = String::ToNumber<int>(ex, temp);
		if (!ex)
			return true;
	}
	return false;
}
bool Parameters::getBool(const string& key, bool& value) const {
	string temp;
	if (!getRaw(key, temp))
		return false;
	Exception ex;
	int n = String::ToNumber<int>(ex, temp);
	value = !ex && (n > 0); // true if n > 0
	value |= (stricmp(temp.c_str(), "false") != 0 && stricmp(temp.c_str(), "no") != 0 && stricmp(temp.c_str(), "off") != 0);
	return true;
}

} // namespace Mona
