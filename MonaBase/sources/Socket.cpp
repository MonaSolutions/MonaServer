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


#include "Mona/Socket.h"

using namespace std;

namespace Mona {


class SocketImpl : public SocketFile {
public:
	void setSendBufferSize(Exception& ex,int size) { setOption(ex, SOL_SOCKET, SO_SNDBUF, size); }
	int  getSendBufferSize(Exception& ex) const { return getOption(ex,SOL_SOCKET, SO_SNDBUF); }
	
	void setReceiveBufferSize(Exception& ex, int size) { setOption(ex, SOL_SOCKET, SO_RCVBUF, size); }
	int  getReceiveBufferSize(Exception& ex) const { return getOption(ex,SOL_SOCKET, SO_RCVBUF); }

	void setNoDelay(Exception& ex, bool flag) { setOption(ex,IPPROTO_TCP, TCP_NODELAY, flag ? 1 : 0); }
	bool getNoDelay(Exception& ex) const { return getOption(ex, IPPROTO_TCP, TCP_NODELAY) != 0; }

	void setKeepAlive(Exception& ex, bool flag) { setOption(ex, SOL_SOCKET, SO_KEEPALIVE, flag ? 1 : 0); }
	bool getKeepAlive(Exception& ex) const { return getOption(ex,SOL_SOCKET, SO_KEEPALIVE) != 0; }

	void setReuseAddress(Exception& ex, bool flag) { setOption(ex, SOL_SOCKET, SO_REUSEADDR, flag ? 1 : 0); }
	bool getReuseAddress(Exception& ex) const { return getOption(ex, SOL_SOCKET, SO_REUSEADDR) != 0; }

	void setOOBInline(Exception& ex, bool flag) { setOption(ex, SOL_SOCKET, SO_OOBINLINE, flag ? 1 : 0); }
	bool getOOBInline(Exception& ex) const { return getOption(ex, SOL_SOCKET, SO_OOBINLINE) != 0; }

	void setLinger(Exception& ex,bool on, int seconds) {
		struct linger l;
		l.l_onoff  = on ? 1 : 0;
		l.l_linger = seconds;
		setOption<struct linger>(ex,SOL_SOCKET, SO_LINGER, l);
	}
	bool getLinger(Exception& ex, int& seconds) const {
		struct linger l;
		getOption<struct linger>(ex, SOL_SOCKET, SO_LINGER, l);
		seconds = l.l_linger;
		return l.l_onoff != 0;
	}

	
	void setReusePort(bool flag) {
#ifdef SO_REUSEPORT
		Exception ex; // ignore error, since not all implementations support SO_REUSEPORT, even if the macro is defined
		setOption(ex,SOL_SOCKET, SO_REUSEPORT, flag ? 1 : 0);
#endif
	}
	bool getReusePort() const {
#ifdef SO_REUSEPORT
		Exception ex;
		return getOption(ex,SOL_SOCKET, SO_REUSEPORT) != 0;
#else
		return false;
#endif
	}

	SocketAddress& address(Exception& ex, SocketAddress& address) const {
		ASSERT_RETURN(_initialized, address)
		union {
			struct sockaddr_in  sa_in;
			struct sockaddr_in6 sa_in6;
		} addr;
		NET_SOCKLEN addrSize = sizeof(addr);
		if (::getsockname(_sockfd,(sockaddr*)&addr, &addrSize) == 0) {
			address.set((sockaddr&)addr);
			return address;
		}
		Net::SetError(ex);
		return address;
	}

	
	SocketAddress& peerAddress(Exception& ex, SocketAddress& address) const {
		ASSERT_RETURN(_initialized, address);
		union {
			struct sockaddr_in  sa_in;
			struct sockaddr_in6 sa_in6;
		} addr;
		NET_SOCKLEN addrSize = sizeof(addr);
		if (::getpeername(_sockfd, (sockaddr*)&addr, &addrSize) == 0) {
			address.set((sockaddr&)addr);
			return address;
		}
		Net::SetError(ex);
		return address;
	}

	UInt32	available(Exception& ex) const { return ioctl(ex, FIONREAD, 0); }

	bool initialized() { return _initialized; }

	const SocketManager&		manager;
	const Socket::Type			type;

	
	SocketImpl(Socket& socket,const SocketManager& manager,Socket::Type type) :
		_pSocket(&socket),
		type(type),
		_initialized(false),
		_connecting(false),
		_pManagedSocket(NULL),
		manager(manager),
		_writing(false),
		SocketFile(NET_INVALID_SOCKET) {
	}

	// initialized and connected socket
	SocketImpl(Socket& socket,SocketFile& file,const SocketManager& manager) :
		_pSocket(&socket),
		type(Socket::STREAM),
		_initialized(true),
		_connecting(false),
		_pManagedSocket(NULL),
		manager(manager),
		_writing(false),
		SocketFile(file._sockfd) {

		file._sockfd = NET_INVALID_SOCKET;
		setNoSigPipe();
	}

	~SocketImpl() {
		if (_sockfd == NET_INVALID_SOCKET)
			return;
		if(_pManagedSocket)
			manager.remove(_sockfd);
		NET_CLOSESOCKET(_sockfd);
	}

	void release() {
		lock_guard<recursive_mutex> lock(_mutexManaged);
		if (!_pSocket)
			return;
		if (_pManagedSocket) {
			manager.remove(_sockfd);
			_pManagedSocket = NULL;
		}
		_pSocket = NULL;
		lock_guard<mutex>	lockSenders(_mutexAsync);
		_senders.clear();
		_connecting = false;
	}

	// Can be called by a separated thread (socketmanager handle thread)
	bool onConnection() {
		lock_guard<mutex> lock(_mutexAsync);
		bool result(_connecting);
		_connecting = false;
		return result;
	}
	
	NET_SOCKET acceptConnection(Exception& ex,SocketAddress& address) {
		ASSERT_RETURN(_initialized, NET_INVALID_SOCKET)

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
			return NET_INVALID_SOCKET;
		}
		address.set((sockaddr&)addr);
		return sockfd;
	}

	

	bool connect(Exception& ex, const SocketAddress& address,bool allowBroadcast) {
		if (!_initialized && !init(ex, address.family()))
			return false;

		ioctl(ex,FIONBIO, 1); // set non blocking mode before bind

		if (allowBroadcast && address.host().isAnyBroadcast())
			setOption(ex, SOL_SOCKET, SO_BROADCAST,1); // ex is warning here
	
		int rc = ::connect(_sockfd, address.addr(), address.size());

		if (rc) {
			int err = Net::LastError();
			if (err != NET_EINPROGRESS && err != NET_EWOULDBLOCK) {
				Net::SetError(ex, err, address.toString());
				return false;
			} else {
				lock_guard<mutex> lock(_mutexAsync);
				_connecting = true;
			}
		}
		managed(ex); // warning
		return true;
	}


	bool bind(Exception& ex, const SocketAddress& address, bool reuseAddress) {
		if (!_initialized && !init(ex, address.family()))
			return false;

		ioctl(ex,FIONBIO, 1); // set non blocking mode before bind

		// TODO? if (address.family() == IPAddress::IPv6)
			// setOption(ex, IPPROTO_IPV6, IPV6_V6ONLY, 1);
		if (reuseAddress) {
			setReuseAddress(ex,true);
			setReusePort(true);
		}
		int rc = ::bind(_sockfd, address.addr(), address.size());
		if (rc != 0) {
			Net::SetError(ex, Net::LastError(), address.toString());
			return false;
		}
		managed(ex); // warning
		return true;
	}

	bool bindWithListen(Exception& ex, const SocketAddress& address, bool reuseAddress,int backlog) {
		if (!_initialized && !init(ex, address.family()))
			return false;
	
	
		// TODO? if (address.family() == IPAddress::IPv6)
			// setOption(ex, IPPROTO_IPV6, IPV6_V6ONLY, 1);
		ioctl(ex,FIONBIO, 1); // set non blocking mode before bind

		if (reuseAddress) {
			setReuseAddress(ex,true);
			setReusePort(true);
		}
		int rc = ::bind(_sockfd, address.addr(), address.size());
		if (rc != 0) {
			Net::SetError(ex, Net::LastError(), address.toString());
			return false;
		}

		if (::listen(_sockfd, backlog) != 0) {
			Net::SetError(ex);
			return false;
		}

		managed(ex); // warning
		return true;
	}


	void shutdown(Exception& ex,Socket::ShutdownType type) {
		ASSERT(_initialized)
		if (::shutdown(_sockfd, type) != 0)
			Net::SetError(ex);
	}


	int sendBytes(Exception& ex, const void* buffer, int length, int flags) {
		ASSERT_RETURN(_initialized, 0);

		int rc;
		do {
			rc = ::send(_sockfd, reinterpret_cast<const char*>(buffer), length, flags);
		} while (rc < 0 && Net::LastError() == NET_EINTR);
		if (rc < 0) {
			int err = Net::LastError();
			if (err == NET_EAGAIN || err == NET_EWOULDBLOCK)
				return 0;
			Net::SetError(ex, err);
		}
		return rc;
	}


	int receiveBytes(Exception& ex, void* buffer, int length, int flags) {
		ASSERT_RETURN(_initialized, 0);
		int rc;
		do {
			rc = ::recv(_sockfd, reinterpret_cast<char*>(buffer), length, flags);
		} while (rc < 0 && Net::LastError() == NET_EINTR);
		if (rc < 0) {
			int err = Net::LastError();
			if (err == NET_EAGAIN || err == NET_EWOULDBLOCK)
				return 0;
			Net::SetError(ex, err);
		}
		return rc;
	}


	int sendTo(Exception& ex, const void* buffer, int length, const SocketAddress& address, bool allowBroadcast,int flags) {
		if (!_initialized && !init(ex, address.family()))
			return 0;

		managed(ex); // ex is warning here

		if (allowBroadcast && address.host().isAnyBroadcast())
			setOption(ex, SOL_SOCKET, SO_BROADCAST,1); // ex is warning here

		int rc;
		do {
			rc = ::sendto(_sockfd, (const char*)buffer, length, flags, address.addr(), address.size());
		} while (rc < 0 && Net::LastError() == NET_EINTR);
		if (rc < 0) {
			int err = Net::LastError();
			if (err == NET_EAGAIN || err == NET_EWOULDBLOCK)
				return 0;
			Net::SetError(ex, err);
		}
		return rc;
	}


	int receiveFrom(Exception& ex, void* buffer, int length, SocketAddress& address, int flags) {
		if (!_initialized && !init(ex, address.family()))
			return 0;

		union {
			struct sockaddr_in  sa_in;
			struct sockaddr_in6 sa_in6;
		} addr;
		NET_SOCKLEN addrSize = sizeof(addr);
		int rc;
		do {
			rc = ::recvfrom(_sockfd, reinterpret_cast<char*>(buffer), length, flags, (sockaddr*)&addr, &addrSize);
		} while (rc < 0 && Net::LastError() == NET_EINTR);
		if (rc < 0) {
			int err = Net::LastError();
			if (err == NET_EAGAIN || err == NET_EWOULDBLOCK)
				return 0;
			Net::SetError(ex, err);
		}
		address.set((sockaddr&)addr);
		return rc;
	}


	bool canSend(Exception& ex) {
		lock_guard<mutex> lock(_mutexAsync);
		return !_connecting && _senders.empty();
	}
	bool addSender(Exception& ex,const shared_ptr<SocketSender>& pSender) {
		lock_guard<recursive_mutex>	lock(_mutexManaged);
		if (!managed(ex)) // check init already, so _sockfd is good!
			return false;
		lock_guard<mutex> lockAsync(_mutexAsync);
		_senders.emplace_back(pSender);
		return _writing = manager.startWrite(_sockfd,_pManagedSocket);
	}

		
	// Is called from one other thread than main thread (by the manager socket thread, so here socket is necessary managed, and _sockfd is good)
	void flushSenders(Exception& ex) {
		lock_guard<recursive_mutex>	lock(_mutexManaged);
		if (!_pManagedSocket)
			return;
		lock_guard<mutex>	lockAsync(_mutexAsync);
		while (!_senders.empty()) {
			if (!_senders.front()->flush(ex,*_pSocket)) {
				if (!_writing)
					_writing = manager.startWrite(_sockfd,_pManagedSocket);
				return;
			}
			_senders.pop_front();
		}
		if (_writing)
			_writing = !manager.stopWrite(_sockfd,_pManagedSocket);
	}

	
	bool managed(Exception& ex) {
		ASSERT_RETURN(_initialized, false);
		lock_guard<recursive_mutex>	lock(_mutexManaged);
		if (_pManagedSocket)
			return true;
		ASSERT_RETURN(_pSocket!=NULL,false)
		ioctl(ex,FIONBIO, 1); // set non blocking mode (usefull for posix)
		if (ex)
			return false;
		return (_pManagedSocket=manager.add(ex,_sockfd, *_pSocket))!=NULL;
	}
	

private:

	bool init(Exception& ex, IPAddress::Family family) {
		lock_guard<mutex>	lock(_mutexInit);
		if (_initialized)
			return true;
		if (!Net::InitializeNetwork(ex))
			return false;
		_sockfd = ::socket(family == IPAddress::IPv6 ? AF_INET6 : AF_INET, type, 0);
		if (_sockfd == NET_INVALID_SOCKET)
			return false;
		setNoSigPipe();
		return _initialized=true;
	}

	void setNoSigPipe() {
#if defined(__MACH__) && defined(__APPLE__) || defined(__FreeBSD__)
		// SIGPIPE sends a signal that if unhandled (which is the default)
		// will crash the process. This only happens on UNIX, and not Linux.
		//
		// In order to have sockets behave the same across platforms, it is
		// best to just ignore SIGPIPE all together.
		Exception ex;
		setOption(ex,SOL_SOCKET, SO_NOSIGPIPE, 1);
#endif
	}
	
	template<typename Type>
	Type& getOption(Exception& ex, int level, int option, Type& value) const {
		ASSERT_RETURN(_initialized, value);
        NET_SOCKLEN length(sizeof(value));
		if (::getsockopt(_sockfd, level, option, reinterpret_cast<char*>(&value), &length) == -1)
			Net::SetError(ex);
		return value;
	}
	int	getOption(Exception& ex, int level, int option) const { int value; return getOption<int>(ex, level, option, value); }

	template<typename Type>
	void setOption(Exception& ex, int level, int option, const Type& value) {
		ASSERT(_initialized);
        NET_SOCKLEN length(sizeof(value));
		if (::setsockopt(_sockfd, level, option, reinterpret_cast<const char*>(&value), length) == -1)
			Net::SetError(ex);
	}
	void setOption(Exception& ex, int level, int option, int value) { setOption<int>(ex, level, option, value); }


	int  ioctl(Exception& ex, NET_IOCTLREQUEST request, int value) const {
		ASSERT_RETURN(_initialized, value);
		return Socket::IOCTL(ex, _sockfd, request, value);
	}
	
	recursive_mutex					_mutexManaged;
	Socket*							_pSocket;
	Socket**						_pManagedSocket;

	mutex							_mutexAsync;
	bool							_writing;
	deque<shared_ptr<SocketSender>>	_senders;
	volatile bool					_connecting;

	mutex							_mutexInit;
	volatile bool					_initialized; // to protect _sockfd access
};






Socket::Socket(SocketEvents& events, const SocketManager& manager, Type type) : _owner(true),_events(events), _pImpl(new SocketImpl(*this,manager,type)) {}
Socket::Socket(SocketFile& file,SocketEvents& events,const SocketManager& manager) : _owner(true),_events(events), _pImpl(new SocketImpl(*this,file,manager)) {
	Exception ex; // TODO? ignore the error on the level, connected and initialized should be manageable
	_pImpl->managed(ex);
}
Socket::Socket(const Socket& other) : _owner(false),_events(other._events), _pImpl(other._pImpl) {}


Socket::~Socket() {
	if (_owner && _pImpl->initialized())
		_pImpl->release();
}

void Socket::close() {
	if (!_pImpl->initialized())
		return;
	if (_owner)
		_pImpl->release();
	_pImpl.reset(new SocketImpl(*this, _pImpl->manager, _pImpl->type));
}

void Socket::onError(const Exception& ex) { _events.onError(ex); }
void Socket::onReadable(Exception& ex,UInt32 available) { _events.onReadable(ex,available); }
bool Socket::onConnection() { return _pImpl->onConnection(); }

bool		Socket::canSend(Exception& ex) { return _pImpl->canSend(ex); }
bool		Socket::addSender(Exception& ex, std::shared_ptr<SocketSender> pSender) { return _pImpl->addSender(ex,pSender); }
void		Socket::flushSenders(Exception& ex) {return _pImpl->flushSenders(ex);}

SocketFile	Socket::acceptConnection(Exception& ex,SocketAddress& address) { return SocketFile(_pImpl->acceptConnection(ex,address)); }
UInt32	 Socket::available(Exception& ex) const { return _pImpl->available(ex); }
bool Socket::connect(Exception& ex, const SocketAddress& address,bool allowBroadcast) { return _pImpl->connect(ex,address,allowBroadcast); }
bool Socket::bind(Exception& ex, const SocketAddress& address, bool reuseAddress) { return _pImpl->bind(ex,address,reuseAddress); }
bool Socket::bindWithListen(Exception& ex, const SocketAddress& address, bool reuseAddress,int backlog)  { return _pImpl->bindWithListen(ex,address,reuseAddress,backlog); }
void Socket::shutdown(Exception& ex, ShutdownType type) { return _pImpl->shutdown(ex,type); }
	
int Socket::receiveBytes(Exception& ex, void* buffer, int length, int flags) { return _pImpl->receiveBytes(ex,buffer,length,flags); }
int	Socket::receiveFrom(Exception& ex, void* buffer, int length, SocketAddress& address, int flags) { return _pImpl->receiveFrom(ex,buffer,length,address,flags); }

int Socket::sendBytes(Exception& ex, const void* buffer, int length, int flags)  { return _pImpl->sendBytes(ex,buffer,length,flags); }
int	Socket::sendTo(Exception& ex, const void* buffer, int length, const SocketAddress& address, bool allowBroadcast, int flags)  { return _pImpl->sendTo(ex,buffer,length,address,allowBroadcast,flags); }


const SocketManager& Socket::manager() const { return _pImpl->manager; }

SocketAddress& Socket::address(Exception& ex, SocketAddress& address) const { return _pImpl->address(ex,address); }
SocketAddress& Socket::peerAddress(Exception& ex, SocketAddress& address) const { return _pImpl->peerAddress(ex,address); }

void Socket::setSendBufferSize(Exception& ex,int size) { _pImpl->setSendBufferSize(ex,size); }
int  Socket::getSendBufferSize(Exception& ex) const { return _pImpl->getSendBufferSize(ex); }

void Socket::setReceiveBufferSize(Exception& ex,int size) { _pImpl->setReceiveBufferSize(ex,size); }
int  Socket::getReceiveBufferSize(Exception& ex) const { return _pImpl->getReceiveBufferSize(ex); }

void Socket::setNoDelay(Exception& ex,bool flag) { _pImpl->setNoDelay(ex,flag); }
bool Socket::getNoDelay(Exception& ex) const { return _pImpl->getNoDelay(ex); }

void Socket::setKeepAlive(Exception& ex,bool flag) { _pImpl->setKeepAlive(ex,flag); }
bool Socket::getKeepAlive(Exception& ex) const { return _pImpl->getKeepAlive(ex); }

void Socket::setReuseAddress(Exception& ex,bool flag) { _pImpl->setReuseAddress(ex,flag); }
bool Socket::getReuseAddress(Exception& ex) const { return _pImpl->getReuseAddress(ex); }

void Socket::setOOBInline(Exception& ex,bool flag) { _pImpl->setOOBInline(ex,flag); }
bool Socket::getOOBInline(Exception& ex) const { return _pImpl->getOOBInline(ex); }

void Socket::setLinger(Exception& ex,bool on, int seconds) { _pImpl->setLinger(ex,on,seconds); }
bool Socket::getLinger(Exception& ex, int& seconds) const { return _pImpl->getLinger(ex,seconds); }

void Socket::setReusePort(bool flag) { _pImpl->setReusePort(flag); }
bool Socket::getReusePort() const  { return _pImpl->getReusePort(); }

int Socket::IOCTL(Exception& ex,NET_SOCKET sockfd,NET_IOCTLREQUEST request,int value) {
	ASSERT_RETURN(sockfd!=NET_INVALID_SOCKET, value)
#if defined(_WIN32)
	int rc = ioctlsocket(sockfd, request, reinterpret_cast<u_long*>(&value));
#else
	int rc = ::ioctl(sockfd, request, &value);
#endif
	if (rc != 0)
		Net::SetError(ex);
	return value;
}

} // namespace Mona
