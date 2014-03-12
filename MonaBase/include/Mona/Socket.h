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

#pragma once

#include "Mona/Mona.h"
#include "Mona/SocketAddress.h"
#include "Mona/SocketManager.h"
#include "Mona/SocketSender.h"
#include "Mona/PoolThreads.h"
#include "Mona/PoolBuffers.h"
#include <memory>
#include <deque>

namespace Mona {



class SocketEvents : virtual Object {
public:
	// Can be called by a separated thread!
	virtual void onError(const std::string& error) = 0;
	// Can be called by a separated thread!
	// if ex of onReadable is raised, it's given to onError
	virtual void onReadable(Exception& ex) = 0;
	// allow to reject a connection on the address client choice (before client object creation)
	virtual bool onConnection(const SocketAddress& address) { return true; }

	virtual Socket&	socket(std::shared_ptr<Socket>& pSocket) = 0;
};


class Socket : private SocketEvents, virtual Object {
	friend class SocketManager;
public:
	enum SocketType {
		STREAM = SOCK_STREAM,
		DATAGRAM = SOCK_DGRAM
	};

	enum ShutdownType {
		RECV = 0,
		SEND = 1,
		BOTH = 2
	};

	// Creates a Socket
	Socket(const SocketManager& manager, SocketEvents& events,SocketType type = STREAM);
	// Destroys the Socket (Closes the socket if it is still open)
	virtual ~Socket();

	const SocketManager&	manager;
	const SocketType		type;

	void release();

	int	available(Exception& ex) { return ioctl(ex, FIONREAD, 0); }
	
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

	bool connect(Exception& ex, const SocketAddress& address,bool allowBroadcast=false);
	bool bind(Exception& ex, const SocketAddress& address, bool reuseAddress = true);
	bool bindWithListen(Exception& ex, const SocketAddress& address, bool reuseAddress = true,int backlog = 64);
	void shutdown(Exception& ex, ShutdownType type = BOTH);
	
	int receiveBytes(Exception& ex, void* buffer, int length, int flags = 0);
	int receiveFrom(Exception& ex, void* buffer, int length, SocketAddress& address, int flags = 0);

	int sendBytes(Exception& ex, const void* buffer, int length, int flags = 0);
	int sendTo(Exception& ex, const void* buffer, int length, const SocketAddress& address, bool allowBroadcast = false, int flags = 0);

	void rejectConnection();

	template<typename SocketType, typename ...Args>
	SocketType* acceptConnection(Exception& ex, Args&&... args) {
		ASSERT_RETURN(_sockfd!=NET_INVALID_SOCKET, NULL)

		union {
			struct sockaddr_in  sa_in;
			struct sockaddr_in6 sa_in6;
		} addr;
		NET_SOCKLEN addrSize = sizeof(addr);
		NET_SOCKET sockfd;
		do {
			sockfd = ::accept(_sockfd, (sockaddr*)&addr, &addrSize);  // TODO acceptEx?
		} while (sockfd == NET_INVALID_SOCKET && Net::LastError() == NET_EINTR);
		if (sockfd == NET_INVALID_SOCKET) {
			Net::SetError(ex);
			return NULL;
		}
		SocketAddress address((sockaddr&)addr);
		{
			std::lock_guard<std::recursive_mutex> lock(_mutexManaged);
			if (_pEvents && !_pEvents->onConnection(address)) {
				NET_CLOSESOCKET(sockfd);
				return NULL;
			}
		}
		SocketType* pSocketType = new SocketType(address,manager,args ...);
		std::shared_ptr<Socket> pSocket;
		Socket& socket = ((SocketEvents*)pSocketType)->socket(pSocket);

		std::lock_guard<std::mutex>	lock(socket._mutexInit);
		socket._sockfd = sockfd;
		if (!socket.managed(ex)) {
			delete pSocketType;
			pSocketType = NULL;
		} else
			socket.setNoDelay(ex, true); // enabe nodelay per default: OSX really needs that
		return pSocketType;
	}
	
	
	// Can be called from one other thread than main thread (by the poolthread)
	template<typename SocketSenderType>
	bool send(Exception& ex,const std::shared_ptr<SocketSenderType>& pSender) {
		// return if no data to send
		if (!pSender->available())
			return true;

		ASSERT_RETURN(_sockfd!=NET_INVALID_SOCKET,false)

		// We can write immediatly if there are no queue packets to write,
		// and if it remains some data to write (flush returns false)
		std::lock_guard<std::mutex>	lock(_mutexAsync);
		if ((_connecting || !_senders.empty()) && pSender->buffering(manager.poolBuffers) || !pSender->flush(ex, *this)) {
			if (managed(ex)) {
				_senders.emplace_back(pSender);
				_writing = manager.startWrite(*this);
			}
			return !ex;
		}
		return true;
	}

private:
	// SocketEvents implementation
	void onError(const std::string& error);
	void onReadable(Exception& ex);
	virtual Socket&	socket(std::shared_ptr<Socket>& pSocket) {return *this;}

	// for the fakesocket of SocketManager
	Socket(const SocketManager& manager);

	
	// Creates the underlying native socket
	bool	init(Exception& ex, IPAddress::Family family);


	bool    managed(Exception& ex);

	// flush async sending
	void	flushSenders(Exception& ex);

	template<typename Type>
	Type& getOption(Exception& ex, int level, int option, Type& value) {
		ASSERT_RETURN(_sockfd!=NET_INVALID_SOCKET, value);
        NET_SOCKLEN length(sizeof(value));
		if (::getsockopt(_sockfd, level, option, reinterpret_cast<char*>(&value), &length) == -1)
			Net::SetError(ex);
		return value;
	}
	int		getOption(Exception& ex, int level, int option) { int value; return getOption<int>(ex, level, option, value); }

	template<typename Type>
	void setOption(Exception& ex, int level, int option, const Type& value) {
		ASSERT(_sockfd!=NET_INVALID_SOCKET);
        NET_SOCKLEN length(sizeof(value));
		if (::setsockopt(_sockfd, level, option, reinterpret_cast<const char*>(&value), length) == -1)
			Net::SetError(ex);
	}
	void	setOption(Exception& ex, int level, int option, int value) { setOption<int>(ex, level, option, value); }

	// A wrapper for the ioctl system call
	int		ioctl(Exception& ex,NET_IOCTLREQUEST request,int value);

	std::unique_ptr<Socket>*					_ppSocket; // deleted by the socketmanager, and exists when managed!

	std::mutex									_mutexAsync;
	bool										_writing;
	std::deque<std::shared_ptr<SocketSender>>	_senders;

	std::recursive_mutex	_mutexManaged;
	volatile bool			_managed;
	SocketEvents*			_pEvents;

	NET_SOCKET				_sockfd;
	std::mutex				_mutexInit;

	volatile bool			_connecting;
};



} // namespace Mona
