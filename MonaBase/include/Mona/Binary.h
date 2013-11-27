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

namespace Mona {

class Binary : virtual Static {
public:
	static UInt16	Flip16(UInt16 value) { return ((value >> 8) & 0x00FF) | ((value << 8) & 0xFF00); }
	static UInt32	Flip24(UInt32 value) { return ((value >> 16) & 0x000000FF) | (value & 0x0000FF00) | ((value << 16) & 0x00FF0000); }
	static UInt32	Flip32(UInt32 value) { return ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000); }
	static UInt64	Flip64(UInt64 value) { UInt32 hi = UInt32(value >> 32); UInt32 lo = UInt32(value & 0xFFFFFFFF); return UInt64(Flip32(hi)) | (UInt64(Flip32(lo)) << 32); }

};


} // namespace Mona
