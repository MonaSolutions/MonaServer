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
	_size(size),WorkThread(name),Task(invoker), _pBuffer(invoker.poolBuffers,size) {
	memcpy(_pBuffer->data(), data,size);
	_current = _pBuffer->data();
}

Decoding::Decoding(const char* name,Invoker& invoker,PoolBuffer& pBuffer) :
	_pBuffer(invoker.poolBuffers),WorkThread(name),Task(invoker),_size(pBuffer->size()),_current(pBuffer->data()) {
	_pBuffer.swap(pBuffer);
}

const UInt8* Decoding::decodeRaw(Exception& ex, PoolBuffer& pBuffer, UInt32 times,const UInt8* data,UInt32& size) {
	PacketReader packet(data,size);
	bool result = decode(ex,packet,times);
	if (!result)
		return NULL;
	size = packet.available();
	return packet.current();
}

bool Decoding::run(Exception& exc) {
	UInt32 times(0);
	UInt32 size(_size);
	while(_size>0) {
		Exception ex;
		if (!(_current = decodeRaw(ex, _pBuffer, times++, _current, _size))) {
			if (ex)
				ERROR(name,", ",ex.error())
			break;
		}
		if (ex)
			WARN(name,", ",ex.error())
		waitHandle();

		_current += _size;
		size -= _size;
		_size = size;
	}
	return true;
}

void Decoding::handle(Exception& ex) {
	unique_lock<mutex> lock;
	Session* pSession = _expirableSession.safeThis(lock);
	if (!pSession)
		return;
	PacketReader packet(_current, _size);
	if (_address.host().isWildcard())
		pSession->receive(packet);
	else
		pSession->receive(packet, _address);
}


} // namespace Mona
