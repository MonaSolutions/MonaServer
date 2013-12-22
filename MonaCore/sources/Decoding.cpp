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

#include "Mona/Decoding.h"
#include "Mona/Session.h"


using namespace std;

namespace Mona {

Decoding::Decoding(const char* name,Invoker& invoker,const UInt8* data,UInt32 size) :
	_size(size),_noFlush(false),_flush(false),WorkThread(name),Task(invoker), _pBuffer(invoker.poolBuffers,size) {
	memcpy(_pBuffer->data(), data,size);
	_current = _pBuffer->data();
}

const UInt8* Decoding::decodeRaw(Exception& ex, PoolBuffer& pBuffer, UInt32 times,const UInt8* data,UInt32& size) {
	if (!_pReader)
		_pReader.reset(new MemoryReader(data,size));
	bool result = decode(ex,*_pReader,times);
	if (!result)
		return NULL;
	return data;
}

bool Decoding::run(Exception& exc) {
	Exception ex;
	UInt32 times(0);
	UInt32 size(_size);
	while(_size>0 && (_current=decodeRaw(ex, _pBuffer,times++,_current,_size))) {
		if (ex)
			WARN(name,", ",ex.error())
		waitHandle();

		_current += _size;
		_size = size-_size;
	}
	if (ex)
		ERROR(name,", ",ex.error())
	if (!_noFlush && times>0 && _current) {
		_flush = true;
		waitHandle();
	}
	return true;
}

void Decoding::handle(Exception& ex) {
	unique_lock<mutex> lock;
	Session* pSession = _expirableSession.safeThis(lock);
	if (!pSession)
		return;
	if (_flush) {
		pSession->flush();
		return;
	}
	if (!_pReader)
		_pReader.reset(new MemoryReader(_current, _size));
	if (_address.host().isWildcard())
		pSession->receive(*_pReader);
	else
		pSession->receive(*_pReader, _address);
	_pReader->reset();
}


} // namespace Mona
