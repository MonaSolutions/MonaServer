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

#include "Mona/BinaryStream.h"

using namespace std;

namespace Mona {


UInt32 BinaryStream::size() {
	streamoff result = _buffer.pubseekoff(0, ios_base::cur, ios_base::out) - _buffer.pubseekoff(0, ios_base::cur, ios_base::in);
	if (result < 0)
		result = 0;
	return (UInt32)result;
}

void BinaryStream::next(UInt32 count) {
	if (count == 0)
		return;
	streamsize before = width(count);
	(*this) << 'z';
	width(before);
}


} // namespace Mona
