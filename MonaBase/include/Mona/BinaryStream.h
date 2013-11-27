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
#include <sstream>

namespace Mona {


class BinaryBuffer : public std::stringbuf, virtual Object {
public:
	const UInt8*	data()	{ return (const UInt8*)gptr(); }
};


class BinaryStream : virtual Object,public std::iostream {
public:
	BinaryStream() : std::iostream(&_buffer) {}

	bool            empty() { return size() == 0; }
	UInt32			size();
	const UInt8*	data() { return _buffer.data(); }

	void            clear() { _buffer.str(""); std::iostream::clear(); }
	void            resetReading(UInt32 position) { _buffer.pubseekoff(position, std::ios::beg, std::ios_base::in); std::iostream::clear(); }
	void			resetWriting(UInt32 position) { _buffer.pubseekoff(position, std::ios::beg, std::ios_base::out); std::iostream::clear(); }
	void			next(UInt32 count);
private:
	BinaryBuffer	_buffer;
};



} // namespace Mona
