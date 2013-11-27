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
#include "Mona/Logs.h"


using namespace std;


namespace Mona {

SocketSender::SocketSender() :
	_position(0), _data(NULL), _size(0), _memcopied(false) {
}

SocketSender::SocketSender(const UInt8* data, UInt32 size) :
	_position(0), _data((UInt8*)data), _size(size), _memcopied(false) {
}

SocketSender::~SocketSender() {
	if(_memcopied)
		delete [] _data;
}

bool SocketSender::run(Exception& ex) {
	unique_lock<mutex> lock;
	Socket* pSocket = _expirableSocket.safeThis(lock);
	if (!pSocket)
		return true;

	// send
	Exception exc;
	shared_ptr<SocketSender> pThis(_pThis);
	pSocket->send(exc, pThis);
	if (exc.code() != Exception::ASSERT)
		ex.set(exc);
	return true;
}


bool SocketSender::flush(Exception& ex,Socket& socket) {
	if(!available())
		return true;
	
	_position += send(ex,socket,begin() + _position, size() - _position);
	if (ex) {
		// terminate the sender
		_position = size();
		return true;
	}
	// everything has been sent
	if (_position == size())
		return true;
	// if data have been given on SocketSender construction we have to copy data to send it in an async way now
	if (!_memcopied && _data == begin()) {
		_size = _size - _position;
		UInt8* temp = new UInt8[_size]();
		memcpy(temp, _data + _position, _size);
		_data = temp;
		_position = 0;
		_memcopied = true;
	}
	// remains data to send
	return false;
}

} // namespace Mona
