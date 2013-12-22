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

#include "Mona/MapParameters.h"
#include <set>

using namespace std;


namespace Mona {

const MapParameters MapParameters::Null(true);

const string* MapParameters::getRaw(const string& key) const {
	auto it = _map.find(key);
	if (it != _map.end())
		return &it->second;
	return NULL;
}

void MapParameters::setRaw(const string& key, const char* value) {
	if (value)
		_map[key] = value;
	else
		_map.erase(key);
}


} // namespace Mona
