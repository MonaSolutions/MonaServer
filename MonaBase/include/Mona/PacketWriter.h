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
#include "Mona/BinaryWriter.h"
#include <memory>

namespace Mona {

class PacketWriter: public BinaryWriter, virtual NullableObject {
public:
	PacketWriter(const PoolBuffers& poolBuffers) : _ppBuffer(new PoolBuffer(poolBuffers)),BinaryWriter(NULL,0) {}
	PacketWriter() : BinaryWriter(NULL,0) {} // NULL

	UInt8* buffer(UInt32 size) { UInt32 pos(this->size()); next(size); return (UInt8*)data()+pos; }

	BinaryWriter&	clear(UInt32 size = 0) { BinaryWriter::clear(size); if (_ppBuffer && _ppBuffer->empty()) _ppBuffer->release(); return *this; }

	operator bool() const { return _ppBuffer ? true : false; }
private:
	Buffer&	buffer() { return _ppBuffer ? **_ppBuffer : Buffer::Null; }

	std::unique_ptr<PoolBuffer>		_ppBuffer;
};


} // namespace Mona
