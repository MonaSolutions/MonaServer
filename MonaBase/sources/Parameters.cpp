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


bool Parameters::getBoolean(const char* key, bool& value) const {
	const char* temp = getRaw(key);
	if (!temp)
		return false;
	value = !String::IsFalse(temp); // otherwise considerate the value as true
	return true;
}

void Parameters::setIntern(const char* key, const char* value, size_t size) {
	UInt32 bytes(setRaw(key, value, value && size == string::npos ? strlen(value) : size));
	if (!bytes)
		return;
	if (key)
		_bytes += bytes;
	else
		_bytes -= bytes;
	onChange(key, value, size);  // value==NULL means "deletion"
}


} // namespace Mona
