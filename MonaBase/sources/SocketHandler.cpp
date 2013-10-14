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

#include "Mona/SocketHandler.h"
#include "Mona/SocketManager.h"
#include "Mona/SocketSender.h"
#include "Mona/Logs.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {

SocketHandlerBase::SocketHandlerBase(const SocketManager& manager) : _pSocketManaged(NULL),_pSocket(NULL),manager(manager),_write(false),_pClosed(new bool(true)) {
}

SocketHandlerBase::~SocketHandlerBase() {
	closeSocket();
}


Socket*	SocketHandlerBase::openSocket(Socket* pSocket) {
	if(_pSocket)
		closeSocket();
	ScopedLock<FastMutex> lock(_mutex);	
	_pSocket=pSocket;
	if(_pSocket) {
		if(!manager.open(*this)) {
			delete _pSocket;
			_pSocket=NULL; // TODO check all the "openSocket" way in the code!!!
		}
		*_pClosed = false;
	}
	_write = false;
	return _pSocket;
}

// Can be called from one other thread than main thread
void SocketHandlerBase::writeSocket(AutoPtr<SocketSender> sender) {
	ScopedLock<FastMutex> lock(_mutex);
	if(!_pSocket) {
		ERROR("Invalid socket, impossible to write")
		return;
	}
	_pSocket->setBlocking(false); // do just in write case to avoid a multihtreaded connect operation (not do before!)
	if(!_senders.empty() || !sender->flush()) {
		_senders.push_back(sender);
		if(!_write) {
			_write=true;
			manager.startWrite(*this);
		}
	}
}

// Can be called from one other thread than main thread
void SocketHandlerBase::flushSocket() {
	ScopedLock<FastMutex> lock(_mutex);
	if(!_pSocket) {
		ERROR("Invalid socket, impossible to flush")
		return;
	}
	while(!_senders.empty()) {
		if(!_senders.front()->flush()) {
			if(!_write) {
				_write=true;
				manager.startWrite(*this);
			}
			return;
		}
		_senders.pop_front();
	}
	if(_write && _senders.empty()) {
		manager.stopWrite(*this);
		_write=false;
	}
}

void SocketHandlerBase::closeSocket() {
	ScopedLock<FastMutex> lock(_mutex);
	if(!_pSocket)
		return;
	manager.close(*this);
	_senders.clear();
	delete _pSocket;
	_pSocket=NULL;
	*_pClosed = true;

}

} // namespace Mona
