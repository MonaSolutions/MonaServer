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
//#include <atomic>


using namespace std;


namespace Mona {

PoolBuffers::PoolBuffers(UInt16 maximumCapacity) : _maximumCapacity(maximumCapacity) {
	// at least one buffer
	_buffers.emplace_back(new Buffer(0));
}

PoolBuffers::~PoolBuffers() {
	clear(true);
}

void PoolBuffers::clear(bool deleting) {
	lock_guard<mutex> lock(_mutex);
	for(Buffer* pBuffer : _buffers)
		delete pBuffer;
	_buffers.clear();
	// at least one buffer
	if (!deleting)
		_buffers.emplace_back(new Buffer(0));
}

// static atomic_int n;

Buffer* PoolBuffers::beginBuffer(UInt32 size) const {
	Buffer* pBuffer(NULL);
	if (size > _maximumCapacity) {
		pBuffer = new Buffer(size);
	//	printf("New %d %p\n", ++n,pBuffer);
		return pBuffer;
	} else {
		lock_guard<mutex> lock(_mutex);
		pBuffer = _buffers.front();
		_buffers.pop_front();
		// at least one buffer
		if (_buffers.empty()) {
			_buffers.emplace_back(new Buffer(0));
			_lastEmptyTime.update();
		}
	}
	if (size>0)
		pBuffer->resize(size,false);
	// printf("get Buffer %u\n",pBuffer->capacity());
//	printf("Get %d %p\n", ++n,pBuffer);
	return pBuffer;
}

void PoolBuffers::endBuffer(Buffer* pBuffer) const {
	if (pBuffer->capacity() > _maximumCapacity) {
		delete pBuffer;
		return;
	}
	pBuffer->clear(); //to fix clip, and resize to 0
	
	// printf("release Buffer %u\n",pBuffer->capacity());
//	printf("Release %d %p\n", --n,pBuffer);
	_mutex.lock();
	if (_lastEmptyTime.isElapsed(120000)) { // 2 minutes
		_lastEmptyTime.update();
		_mutex.unlock();
		delete pBuffer;
		return;
	}
	_buffers.emplace_back(pBuffer);
	_mutex.unlock();
}


} // namespace Mona
