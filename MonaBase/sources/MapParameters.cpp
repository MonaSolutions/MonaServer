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

using namespace std;


namespace Mona {

UInt32 MapParameters::iteration(const char* prefix, const ForEach& function) const {
	auto it = _map.begin();
	UInt32 prefixSize(0);
	if (prefix) {
		prefixSize = strlen(prefix);
		it = _map.lower_bound(prefix);
	}
	UInt32 count(0);
	while (it != _map.end() && (prefixSize==0 || memcmp(prefix, it->first.c_str(), prefixSize) == 0)) {
		if (prefixSize>0) {
			const std::string key(&it->first[prefixSize]);
			function(key, it->second);
		} else
			function(it->first, it->second);
		++count;
		++it;
	}
	return count;
}

UInt32 MapParameters::setRaw(const char* key, const char* value, UInt32 size) {
	if (!value) {
		auto it(_map.find(key));
		if (it == _map.end())
			return 0;
		size = it->second.size();
		_map.erase(it);
		return size;
	}
	_map[key].assign(value, size);
	return size;
}


} // namespace Mona
