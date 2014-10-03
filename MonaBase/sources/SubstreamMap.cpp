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

#include "Mona/SubstreamMap.h"
#include <algorithm>

using namespace std;

namespace Mona {

SubstreamMap & SubstreamMap::addSub(UInt32 pos, UInt32 size) {

	if (size && pos < _size && pos + size <= _size) {
		_listSubs.emplace_back((UInt8*)_data + pos, size);
		_totalSize += size;
	}
	return *this;
}

UInt32 SubstreamMap::readNextSub(UInt8*& pos, UInt32 maxsize) {

	if (_listSubs.size() <= _index)
		return 0;

	// Get current substream
	pair<UInt8*, UInt32> & sub = _listSubs.at(_index);
	UInt32 offset = 0;
	if (_pos) {
		pos = _pos;
		offset = (_pos-sub.first);
	} else
		pos = sub.first;
	UInt32 readed = min(maxsize, sub.second-offset);

	// Move cursor
	if (readed == sub.second-offset) { // Go to next substream
		_index++;
		_pos = NULL;
	} else
		_pos = pos+readed;

	return readed;
}


} // namespace Mona
