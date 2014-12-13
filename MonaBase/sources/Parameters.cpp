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


bool Parameters::getString(const char* key, std::string& value) const {
	const char* temp = getRaw(key);
	if (!temp)
		return false;
	value.assign(temp);
	return true;
}


bool Parameters::getBool(const char* key, bool& value) const {
	const char* temp = getRaw(key);
	if (temp) {
		if (String::ICompare(temp, "0") == 0 || String::ICompare(temp, "false") == 0 || String::ICompare(temp, "no") == 0 || String::ICompare(temp, "off") == 0) {
			value = false;
			return true;
		}
		if (String::ICompare(temp, "1") == 0 || String::ICompare(temp, "true") == 0 || String::ICompare(temp, "yes") == 0 || String::ICompare(temp, "on") == 0) {
			value = true;
			return true;
		}
	}
	return false;
}

void Parameters::setIntern(const char* key, const char* value, size_t size) {
	UInt32 bytes(setRaw(key, value, value && size == string::npos ? strlen(value) : size));
	if (!bytes)
		return;
	if (key)
		_bytes += bytes;
	else
		_bytes -= bytes;
	if (bytes)
		OnChange::raise(key,value, size); // value==NULL means "deletion"
}


} // namespace Mona
