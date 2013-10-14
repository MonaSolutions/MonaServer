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

#pragma once

#include "Mona.h"
#include "Mona/PoolThread.h"
#include "Poco/AutoPtr.h"
#include "Poco/SharedPtr.h"
#include "Poco/Net/Socket.h"
#include <list>

namespace Mona {

class SocketSender;
class SocketManaged;
class SocketHandlerBase {
	friend class SocketManager;
	friend class SocketSender;
public:
	Poco::Net::Socket*		getSocket();

	const SocketManager&	manager;
protected:
	SocketHandlerBase(const SocketManager& manager);
	virtual ~SocketHandlerBase();

	Poco::Net::Socket*		openSocket(Poco::Net::Socket* pSocket);
	void					closeSocket();

private:
	virtual void			onReadable()=0;
	virtual void			onError(const std::string& error)=0;

	void					writeSocket(Poco::AutoPtr<SocketSender> sender);
	void					flushSocket();

	Poco::Net::Socket*						_pSocket;
	SocketManaged*							_pSocketManaged;
	std::list<Poco::AutoPtr<SocketSender>>	_senders;
	Poco::FastMutex							_mutex;
	bool									_write;
	Poco::SharedPtr<bool>					_pClosed;
};

inline Poco::Net::Socket* SocketHandlerBase::getSocket() {
	Poco::ScopedLock<Poco::FastMutex> lock(_mutex);
	return _pSocket;
}

template<class SocketType>
class SocketHandler : public SocketHandlerBase {
public:
	SocketType*	getSocket() {
		return (SocketType*)SocketHandlerBase::getSocket();
	}
protected:
	SocketHandler(const SocketManager& manager) : SocketHandlerBase(manager){}
	virtual ~SocketHandler() {}

	SocketType* openSocket(SocketType* pSocket) {
		return (SocketType*)SocketHandlerBase::openSocket(pSocket);
	}

};



} // namespace Mona
