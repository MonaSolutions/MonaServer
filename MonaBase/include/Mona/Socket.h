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

#include "Mona/Mona.h"
#include "Mona/SocketAddress.h"
#include "Mona/PoolThread.h"
#include "Mona/SocketSender.h"
#include "Mona/PoolThreads.h"
#include <memory>
#include <list>

namespace Mona {

class SocketManager;
class SocketSender;

class Socket : virtual Object {
	friend class SocketManager;
	friend class SocketSender;
public:

	// Destroys the Socket (Closes the socket if it is still open)
	virtual ~Socket();

	enum ShutdownType {
		RECV = 0,
		SEND = 1,
		BOTH = 2
	};

	int		available(Exception& ex) { return ioctl(ex, FIONREAD, 0); }
	void	close();
	
	SocketAddress& address(Exception& ex, SocketAddress& address) const;
	SocketAddress& peerAddress(Exception& ex, SocketAddress& address) const;


	void setSendBufferSize(Exception& ex,int size) { setOption(ex, SOL_SOCKET, SO_SNDBUF, size); }
	int  getSendBufferSize(Exception& ex) { return getOption(ex,SOL_SOCKET, SO_SNDBUF); }
	
	void setReceiveBufferSize(Exception& ex, int size) { setOption(ex, SOL_SOCKET, SO_RCVBUF, size); }
	int  getReceiveBufferSize(Exception& ex) { getOption(ex,SOL_SOCKET, SO_RCVBUF); }

	void setNoDelay(Exception& ex, bool flag) { setOption(ex,IPPROTO_TCP, TCP_NODELAY, flag ? 1 : 0); }
	bool getNoDelay(Exception& ex) { return getOption(ex, IPPROTO_TCP, TCP_NODELAY) != 0; }

	void setKeepAlive(Exception& ex, bool flag) { setOption(ex, SOL_SOCKET, SO_KEEPALIVE, flag ? 1 : 0); }
	bool getKeepAlive(Exception& ex) { return getOption(ex,SOL_SOCKET, SO_KEEPALIVE) != 0; }

	void setReuseAddress(Exception& ex, bool flag) { setOption(ex, SOL_SOCKET, SO_REUSEADDR, flag ? 1 : 0); }
	bool getReuseAddress(Exception& ex) { return getOption(ex, SOL_SOCKET, SO_REUSEADDR) != 0; }

	void setOOBInline(Exception& ex, bool flag) { setOption(ex, SOL_SOCKET, SO_OOBINLINE, flag ? 1 : 0); }
	bool getOOBInline(Exception& ex) { return getOption(ex, SOL_SOCKET, SO_OOBINLINE) != 0; }

	void setLinger(Exception& ex, bool on, int seconds);
	bool getLinger(Exception& ex, int& seconds);

	void setReusePort(bool flag);
	bool getReusePort();

	template<typename SocketSenderType>
	// Can be called from one other thread than main thread (by the poolthread)
	void send(Exception& ex, std::shared_ptr<SocketSenderType>& pSender) {
		ASSERT(_initialized == true)
		if (!managed(ex))
			return;

		// return if no data to send
		if (!pSender->available())
			return;

		// We can write immediatly if there are no queue packets to write,
		// and if it remains some data to write (flush returns false)
		std::lock_guard<std::mutex>	lock(_mutexAsync);
		if (!_senders.empty() || !pSender->flush(ex, *this)) {
            _senders.push_back(std::static_pointer_cast<SocketSender>(pSender));
			manageWrite(ex);
		}
	}

	template<typename SocketSenderType>
	PoolThread* send(Exception& ex, std::shared_ptr<SocketSenderType>& pSender, PoolThread* pThread) {
		ASSERT_RETURN(_initialized == true, pThread)
		if (!managed(ex))
			return pThread;

		// return if no data to send
		if (!pSender->available())
			return pThread;

		pSender->_pSocket = this;
		pSender->_pSocketMutex = _pMutex;
		pSender->_pThis = pSender;
		pThread = _poolThreads.enqueue<SocketSenderType>(ex,pSender, pThread);
		return pThread;
	}


#if defined(_WIN32)
	static int  LastError() { return WSAGetLastError(); }
#else
	static int  LastError() { return errno; }
#endif

	static bool CheckError(Exception& ex);
	static void SetError(Exception& ex) { SetError(ex, LastError(), std::string()); }
	static void SetError(Exception& ex, int error) { SetError(ex, error, std::string()); }
	static void SetError(Exception& ex, int error, const std::string& argument);

protected:
	// Creates a Socket
	Socket(const SocketManager& manager, int type = SOCK_STREAM);

	template<typename SocketType, typename ...Args>
	SocketType* acceptConnection(Exception& ex, Args&... args) {
		ASSERT_RETURN(_initialized == true, NULL)

		char buffer[IPAddress::MAX_ADDRESS_LENGTH];
		struct sockaddr* pSA = reinterpret_cast<struct sockaddr*>(buffer);
		NET_SOCKLEN saLen = sizeof(buffer);

		NET_SOCKET sockfd;
		do {
			sockfd = ::accept(_sockfd, pSA, &saLen);  // TODO acceptEx?
		} while (sockfd == NET_INVALID_SOCKET && LastError() == EINTR);
		if (sockfd == NET_INVALID_SOCKET) {
			SetError(ex);
			return NULL;
		}
		SocketAddress address;
		if (!address.set(ex, *pSA) || !onConnection(address))
			return NULL;
		SocketType* pSocketType = new SocketType(address,args ...);
		Socket* pSocketBase = reinterpret_cast<Socket*>(pSocketType);
		std::lock_guard<std::mutex>	lock(pSocketBase->_mutexInit);
		if (!pSocketBase->init(ex, sockfd)) {
			delete pSocketType;
			pSocketType = NULL;
		}
		return pSocketType;
	}
	void rejectConnection();

	bool connect(Exception& ex, const SocketAddress& address);
	bool bind(Exception& ex, const SocketAddress& address, bool reuseAddress = true);
	bool listen(Exception& ex, int backlog = 64);

	int receiveBytes(Exception& ex, void* buffer, int length, int flags = 0);
	int receiveFrom(Exception& ex, void* buffer, int length, SocketAddress& address, int flags = 0);


	void shutdown(Exception& ex, ShutdownType type = BOTH);

	int sendBytes(Exception& ex, const void* buffer, int length, int flags = 0);
	int sendTo(Exception& ex, const void* buffer, int length, const SocketAddress& address, int flags = 0);

	void setBroadcast(Exception& ex, bool flag) { setOption(ex, SOL_SOCKET, SO_BROADCAST, flag ? 1 : 0); }
	bool getBroadcast(Exception& ex) { return getOption(ex, SOL_SOCKET, SO_BROADCAST) != 0; }

	const SocketManager&	manager;

	// Can be called by a separated thread!
	virtual void	onError(const std::string& error) = 0;
private:
	virtual bool    onConnection(const SocketAddress& address) { return true; }
	// if ex of onReadable is raised, it's given to onError
	virtual void	onReadable(Exception& ex) = 0;
	

	// Creates the underlying native socket
	bool	init(Exception& ex, IPAddress::Family family);
	bool	init(Exception& ex, NET_SOCKET sockfd);

	bool    managed(Exception& ex);

	// flush async sending
	void    manageWrite(Exception& ex);
	void	flush(Exception& ex);

	template<typename Type>
	Type&	getOption(Exception& ex, int level, int option, Type& value);
	int		getOption(Exception& ex, int level, int option) { int value; return getOption<int>(ex, level, option, value); }

	template<typename Type>
	void	setOption(Exception& ex, int level, int option, const Type& value);
	void	setOption(Exception& ex, int level, int option, int value) { setOption<int>(ex, level, option, value); }

	// A wrapper for the ioctl system call
	int		ioctl(Exception& ex,NET_IOCTLREQUEST request,int value);



	std::shared_ptr<std::mutex>					_pMutex; // used by SocketSender

	std::unique_ptr<Socket>*					_ppSocket; // deleted by the socketmanager, and exists when managed!

	std::mutex									_mutexAsync;
	bool										_writing;
	std::list<std::shared_ptr<SocketSender>>	_senders;

	PoolThreads&								_poolThreads;

	std::mutex				_mutexManaged;
	volatile bool			_managed;

	NET_SOCKET				_sockfd;
	std::mutex				_mutexInit;
	volatile bool			_initialized;
	int						_type;
};



} // namespace Mona
