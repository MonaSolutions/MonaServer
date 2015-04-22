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
#include "Mona/BinaryReader.h"
#include "Mona/PoolBuffer.h"
#include <memory>

namespace Mona {


class PacketReader: public BinaryReader, virtual public NullableObject {
public:
	PacketReader(const UInt8* data, UInt32 size) : BinaryReader(data,size) {}
	PacketReader(PoolBuffer& pBuffer) : _ppBuffer(new PoolBuffer(pBuffer.poolBuffers)), BinaryReader(pBuffer->data(), pBuffer->size()) { _ppBuffer->swap(pBuffer); }

	operator bool() const { return data()!=NULL; }

	static PacketReader Null;

private:
	std::unique_ptr<PoolBuffer>		_ppBuffer;
};


} // namespace Mona
