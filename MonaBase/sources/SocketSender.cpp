/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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

SocketSender::~SocketSender() {
	if(_memcopied)
		delete [] _data;
}

bool SocketSender::run(Exception& ex) {
	if (!_pSocketMutex || !_pSocket || _pThis.expired()) {
		ex.set(Exception::SOCKET, "SocketSender failure, its associated socket is null");
		return false;
	}

	lock_guard<mutex> lock(*_pSocketMutex); // prevent socket deletion and socket close
	if (_pSocketMutex.unique()) // socket is deleted
		return true;
	if (!_pSocket->_managed) // socket no more managed
		return true;
	shared_ptr<SocketSender> pThis(_pThis);
	_pSocket->send(ex, pThis);
	return true;
}

void SocketSender::dump(bool justInDebug) {
	if(!_dump)
		return;
	if (!justInDebug || justInDebug&&Logs::GetLevel() >= 7) {
		string address;
		if (receiver(address) && _pSocket) {
			SocketAddress address;
			Exception ex;
			DUMP(begin(true), size(true), "Response to ", _pSocket->peerAddress(ex, address).toString())
		} else
			DUMP(begin(true), size(true), "Response to ", address)
		
	}
	_dump=false;
}

bool SocketSender::flush(Exception& ex,Socket& socket) {
	if(!available())
		return true;
	dump();

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
