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

#include "Mona/Parameters.h"
#include "Mona/String.h"
#include "Mona/Exceptions.h"

using namespace std;

namespace Mona {


bool Parameters::getString(const std::string& key, std::string& value) const {
	const string* pTemp = getRaw(key);
	if (!pTemp)
		return false;
	value.assign(*pTemp);
	return true;
}


bool Parameters::getBool(const string& key, bool& value) const {
	const string* pTemp = getRaw(key);
	if (pTemp) {
		if (pTemp->empty() || String::ICompare(*pTemp, "0") == 0 || String::ICompare(*pTemp, "false") == 0 || String::ICompare(*pTemp, "no") == 0 || String::ICompare(*pTemp, "off") == 0) {
			value = false;
			return true;
		}
		if (String::ICompare(*pTemp, "1") == 0 || String::ICompare(*pTemp, "true") == 0 || String::ICompare(*pTemp, "yes") == 0 || String::ICompare(*pTemp, "on") == 0) {
			value = true;
			return true;
		}
	}
	return false;
}


} // namespace Mona
