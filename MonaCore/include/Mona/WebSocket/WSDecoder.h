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
#include "Mona/Decoder.h"

namespace Mona {


class WSReader : public virtual Object, public PacketReader {
public:
	WSReader(const UInt8* data,UInt32 size, UInt8 type) : type(type),PacketReader(data,size) {}
	const UInt8		type;
};

class WSDecoder : public Decoder<WSReader>, public virtual Object {
public:
	WSDecoder(Invoker& invoker) : Decoder<WSReader>(invoker,"WSDecoder") {}
private:
	UInt32 decoding(Exception& ex, UInt8* data,UInt32 size);
};


} // namespace Mona
