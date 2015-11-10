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
#include "Mona/PoolBuffer.h"


using namespace std;


namespace Mona {


void PoolBuffers::clear() {
	lock_guard<mutex> lock(_mutex);
	for(const auto& it : _buffers)
		delete it.second;
	_buffers.clear();
}

void PoolBuffers::manage() {
	lock_guard<mutex> lock(_mutex);
	if (_buffers.empty()) {
		_lastEmptyTime.update();
		return;
	}
	if (!_lastEmptyTime.isElapsed(10000)) // 10 sec	
		return;
	// remove the bigger buffer
	auto itBigger(_buffers.end());
	delete (--itBigger)->second;
	_buffers.erase(itBigger);
}

Buffer* PoolBuffers::beginBuffer(UInt32 size) const {
	Buffer* pBuffer(NULL);
	{
		lock_guard<mutex> lock(_mutex);
		// choose the smaller buffer 
		if (_buffers.empty()) {
			pBuffer = new Buffer(size);
			_lastEmptyTime.update();
			size = 0; // not to resize
		} else {
			auto itBigger(size ? _buffers.lower_bound(size) : _buffers.end());
			if (itBigger == _buffers.end())
				--itBigger;
			pBuffer = itBigger->second;
			_buffers.erase(itBigger);
		}
	}
	if (size)
		pBuffer->resize(size,false);
	return pBuffer;
}

void PoolBuffers::endBuffer(Buffer* pBuffer) const {
	pBuffer->clear();
	lock_guard<mutex> lock(_mutex);
	_buffers.emplace(pBuffer->capacity(),pBuffer);
}


} // namespace Mona
