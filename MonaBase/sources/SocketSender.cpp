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

#include "Mona/SocketSender.h"
#include "Mona/Socket.h"


using namespace std;


namespace Mona {

SocketSender::SocketSender(const char* name) : WorkThread(name),
	_position(0), _data(NULL), _size(0) {
}

SocketSender::SocketSender(const char* name,const UInt8* data, UInt32 size) : WorkThread(name),
	_position(0), _data((UInt8*)data), _size(size) {
}

bool SocketSender::run(Exception& ex) {
	if (!_pSocket) {
		ex.set(Exception::SOCKET, "SocketSender ", name, " started in parallel without pointer of socket");
		return false;
	}
	// send
	Exception exc;
	shared_ptr<SocketSender> pThis(_pThis);
	_pSocket->send(exc, pThis);
	if (exc.code() != Exception::ASSERT)
		ex = exc;
	return true;
}


bool SocketSender::flush(Exception& ex,Socket& socket) {
	if (available()) {

		UInt32 size;
		const UInt8* data;
		if (_ppBuffer) {
			size = (*_ppBuffer)->size();
			data = (*_ppBuffer)->data();
		} else {
			size = this->size();
			data = this->data();
		}

		_position += send(ex,socket,data + _position, size - _position);

		if (ex) // terminate the sender if error
			_position = size;
		// everything has been sent
		if (_position >= size) {
			if (_ppBuffer)
				_ppBuffer->release();
		} else if (buffering(socket.manager().poolBuffers))
			return false;
	}
	onSent(socket);
	return true;
}

bool SocketSender::buffering(const PoolBuffers& poolBuffers) {
	// if data have been given on SocketSender construction we have to copy data to send it in an async way now
	if (!_data || _ppBuffer)
		return true; // no buffering required
	if (_position >= _size)
		return false; // no more data to send
	UInt32 size(_size-_position);
	_ppBuffer.reset(new PoolBuffer(poolBuffers, size));
	memmove((*_ppBuffer)->data(), _data + _position, size);
	_position = 0;
	return true;
}

} // namespace Mona
