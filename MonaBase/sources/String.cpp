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

#include "Mona/String.h"

using namespace std;

namespace Mona {

char		String::_Buffer[64];

void String::Split(const std::string& value, const std::string& separators, std::vector<std::string>& values, int options) {
	string::const_iterator it1 = value.begin(), it2, it3, end = value.end();

	while (it1 != end) {
		if (options & TOK_TRIM) {
			while (it1 != end && isblank(*it1))
				++it1;
		}
		it2 = it1;
		while (it2 != end && separators.find(*it2) == string::npos)
			++it2;
		it3 = it2;
		if (it3 != it1 && (options & TOK_TRIM)) {
			--it3;
			while (it3 != it1 && isspace(*it3))
				--it3;
			if (!isblank(*it3))
				++it3;
		}
		if (options & TOK_IGNORE_EMPTY) {
			if (it3 != it1)
				values.emplace_back(it1, it3);
		} else
			values.emplace_back(it1, it3);
		it1 = it2;
		if (it1 != end)
			++it1;
	}
}

const string& String::toLower(string& str) {
	auto it = str.begin();
	for(it;it<str.end();++it)
		*it = tolower(*it);
	
	return str;
}

} // namespace Mona
