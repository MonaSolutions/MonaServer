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
#include <cctype>

using namespace std;

namespace Mona {

vector<string>& String::Split(const string& value, const string& separators, vector<string>& values, int options) {
	string::const_iterator it1 = value.begin(), it2, it3, end = value.end();

	while (it1 != end) {
		if (options & SPLIT_TRIM) {
			while (it1 != end && isspace(*it1))
				++it1;
		}
		it2 = it1;
		while (it2 != end && separators.find(*it2) == string::npos)
			++it2;
		it3 = it2;
		if (it3 != it1 && (options & SPLIT_TRIM)) {
			--it3;
			while (it3 != it1 && isspace(*it3))
				--it3;
			if (!isspace(*it3))
				++it3;
		}
		if (options & SPLIT_IGNORE_EMPTY) {
			if (it3 != it1)
				values.emplace_back(it1, it3);
		} else
			values.emplace_back(it1, it3);
		it1 = it2;
		if (it1 != end)
			++it1;
	}
	return values;
}

string& String::ToLower(string& value) {
	auto it = value.begin();
	for (it; it < value.end(); ++it)
		*it = tolower(*it);
	return value;
}

string& String::Trim(string& value, TrimOption option) {
	int first = 0;
	int last = value.size() - 1;

	if (option & 1) {
		while (first <= last && isspace(value[first]))
			++first;
	}
	if (option & 2) {
		while (last >= first && isspace(value[last]))
			--last;
	}

	value.resize(last + 1);
	value.erase(0, first);
	return value;
}

int String::ICompare(const char* value1, const char* value2, int size) {
	if (value1 == value2)
		return 0;
	if (value1 == NULL)
		return -1;
	if (value2 == NULL)
		return 1;

	int f(0), l(0);
	do {
		if (size == 0)
			return f - l;
		if (((f = (unsigned char)(*(value1++))) >= 'A') && (f <= 'Z'))
			f -= 'A' - 'a';
		if (((l = (unsigned char)(*(value2++))) >= 'A') && (l <= 'Z'))
			l -= 'A' - 'a';
		if (size > 0)
			--size;
	} while (f && (f == l));

	return(f - l);
}



} // namespace Mona
