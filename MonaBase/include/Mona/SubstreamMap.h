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

#pragma once

#include "Mona/Mona.h"
#include <vector>

namespace Mona {


/// \brief Gives the possibility to parse a Binary Stream
/// in substream composed of pairs (pointer/size)
class SubstreamMap : Mona::Object {
public:
	SubstreamMap(const UInt8* data, const UInt32 size) : _totalSize(0), _data(data), _size(size), _index(0), _pos(NULL) {}

	/// \bried Define a substream and add it to the end of the list
	SubstreamMap & addSub(UInt32 pos, UInt32 size);

	/// \return true if there is a next sub and it's position is 0
	bool nextSubIsNew() const { return _listSubs.size() && _pos == NULL; }

	/// \return minimum size between maxsize and size of substream
	/// 0 if there is no more substream available
	UInt32 readNextSub(UInt8*& pos, UInt32 maxsize);

	/// \return Number of substreams
	UInt32 count() const { return _listSubs.size(); }

	/// \return Total size of substreams
	UInt32 totalSize() const { return _totalSize; }

	/// \brief Reset Cursor
	void resetPos() {
		_index = 0;
		_pos = NULL;
	}

	/// \return Original Data
	const UInt8* originalData() const { return _data; } 

	/// \return Original Size
	UInt32 originalSize() const { return _size; } 

private:
	UInt32			_index;
	UInt8*			_pos;
	UInt32			_totalSize;

	const UInt8*	_data;
	const UInt32	_size;

	std::vector<std::pair<UInt8*, UInt32>> _listSubs ;
};


} // namespace Mona
