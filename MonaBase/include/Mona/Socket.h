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

namespace Mona {

class SocketEvents : virtual Object {
public:
	// Can be called by a separated thread!
	virtual void onError(const Exception& ex) = 0;
	// Can be called by a separated thread!
	// if ex of onReadable is raised, it's given to onError
	virtual void onReadable(Exception& ex,UInt32 available) = 0;
};

class SocketFile : virtual NullableObject {
	friend class SocketImpl;
	friend class Socket;
public:
	SocketFile(const SocketFile& other) : _sockfd(other._sockfd) {}
	virtual ~SocketFile() {
		if (_sockfd != NET_INVALID_SOCKET)
			NET_CLOSESOCKET(_sockfd);
	}
	operator bool() const { return _sockfd != NET_INVALID_SOCKET;  }
private:
	SocketFile(NET_SOCKET sockfd) : _sockfd(sockfd) {}
	NET_SOCKET			_sockfd;
};


class SocketImpl;
class Socket : virtual Object {
	friend class SocketManager;
	friend class SocketImpl;
public:
	enum Type {
		STREAM = SOCK_STREAM,
		DATAGRAM = SOCK_DGRAM
	};

	enum ShutdownType {
		RECV = 0,
		SEND = 1,
		BOTH = 2
	};

	// Creates a Socket
	Socket(SocketEvents& events, const SocketManager& manager, Type type = STREAM);
	// Create a socket from other socket
	Socket(const Socket& other);
	// Create a socket from SocketFile
	Socket(SocketFile& file, SocketEvents& events, const SocketManager& manager);

	// Destroys the Socket (Closes the socket if it is still open)
	virtual ~Socket();

	const SocketManager&	manager() const;

	void close();

	UInt32 available(Exception& ex) const;
	
	SocketAddress& address(Exception& ex, SocketAddress& address) const;
	SocketAddress& peerAddress(Exception& ex, SocketAddress& address) const;

	void setSendBufferSize(Exception& ex, int size);
	int  getSendBufferSize(Exception& ex) const;
	
	void setReceiveBufferSize(Exception& ex, int size);
	int  getReceiveBufferSize(Exception& ex) const;

	void setNoDelay(Exception& ex, bool flag);
	bool getNoDelay(Exception& ex) const;

	void setKeepAlive(Exception& ex, bool flag);
	bool getKeepAlive(Exception& ex) const;

	void setReuseAddress(Exception& ex, bool flag);
	bool getReuseAddress(Exception& ex) const;

	void setOOBInline(Exception& ex, bool flag);
	bool getOOBInline(Exception& ex) const;

	void setLinger(Exception& ex, bool on, int seconds);
	bool getLinger(Exception& ex, int& seconds) const;

	void setReusePort(bool flag);
	bool getReusePort() const;

	SocketFile acceptConnection(Exception& ex,SocketAddress& address);

	bool connect(Exception& ex, const SocketAddress& address,bool allowBroadcast=false);
	bool bind(Exception& ex, const SocketAddress& address, bool reuseAddress = true);
	bool bindWithListen(Exception& ex, const SocketAddress& address, bool reuseAddress = true,int backlog = 64);
	void shutdown(Exception& ex, ShutdownType type = BOTH);
	
	int receiveBytes(Exception& ex, void* buffer, int length, int flags = 0);
	int	receiveFrom(Exception& ex, void* buffer, int length, SocketAddress& address, int flags = 0);

	int sendBytes(Exception& ex, const void* buffer, int length, int flags = 0);
	int	sendTo(Exception& ex, const void* buffer, int length, const SocketAddress& address, bool allowBroadcast = false, int flags = 0);

	// Can be called from one other thread than main thread (by the poolthread)
	template<typename SocketSenderType>
	bool send(Exception& ex,const std::shared_ptr<SocketSenderType>& pSender) {
		// return if no data to send
		if (!pSender->available())
			return true;

		// We can write immediatly if there are no queue packets to write,
		// and if it remains some data to write (flush returns false)
		if (canSend(ex) && pSender->flush(ex, *this))
			return true;
		if (ex)
			return false;
		if (pSender->buffering(manager().poolBuffers))
			return addSender(ex,pSender);
		return true;
	}

	template<typename SenderType>
	PoolThread* send(Exception& ex,const std::shared_ptr<SenderType>& pSender, PoolThread* pThread) {
		pSender->_pThis = pSender;
		pSender->_pSocket.reset(new Socket(*this));
		pThread = manager().poolThreads.enqueue<SenderType>(ex,pSender, pThread);
		return pThread;
	}

	
private:
	bool canSend(Exception& ex);
	bool addSender(Exception& ex, std::shared_ptr<SocketSender> pSender);
	void flushSenders(Exception& ex);

	// just for SocketManager class!
	void onError(const Exception& ex);
	void onReadable(Exception& ex,UInt32 available);
	bool onConnection();
	

	static int IOCTL(Exception& ex, NET_SOCKET sockfd, NET_IOCTLREQUEST request, int value);

	
	SocketEvents&				  _events;
	const bool					  _owner;
	std::shared_ptr<SocketImpl>   _pImpl;

};


} // namespace Mona
