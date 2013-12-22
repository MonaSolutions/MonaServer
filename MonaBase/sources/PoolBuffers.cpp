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

#include "Mona/PoolBuffers.h"


using namespace std;


namespace Mona {


void PoolBuffers::clear() {
	lock_guard<mutex> lock(_mutex);
	for (Buffer* pBuffer : _buffers)
		delete pBuffer;
	_buffers.clear();
}

Buffer* PoolBuffers::beginBuffer(UInt32 size) const {
	Buffer* pBuffer(NULL);
	{
		lock_guard<mutex> lock(_mutex);
		if (size > _maximumBufferSize || _buffers.empty())
			return new Buffer(size);
		pBuffer = _buffers.front();
		_buffers.pop_front();
	}
	if (size > 0 && size != pBuffer->size())
		pBuffer->resize(size);
	return pBuffer;
}

void PoolBuffers::endBuffer(Buffer* pBuffer) const {
	if (pBuffer->capacity() > _maximumBufferSize) {
		delete pBuffer;
		return;
	}
	lock_guard<mutex> lock(_mutex);
	_buffers.emplace_back(pBuffer);
}


} // namespace Mona
