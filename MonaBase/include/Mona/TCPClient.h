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
#include "Mona/Socket.h"

namespace Mona {

class TCPClient;
namespace Events {
	// Can be called by a separated thread!
	struct OnData : Event<UInt32(PoolBuffer&)> {};
	struct OnDisconnection : Event<void(TCPClient&,const SocketAddress&)> {};
};

class TCPClient : public virtual Object,
	public Events::OnError,
	public Events::OnData,
	public Events::OnDisconnection {
public:
	TCPClient(const SocketManager& manager);
	TCPClient(const SocketAddress& peerAddress,SocketFile& file,const SocketManager& manager);
	virtual ~TCPClient();

	UInt32					idleTime() { std::lock_guard<std::mutex> lock(_mutexIdleTime); return (UInt32)_idleTime.elapsed(); }

	// unsafe-threading
	const SocketAddress&	address() const { std::lock_guard<std::mutex> lock(_mutex); return updateAddress(); }
	const SocketAddress&	peerAddress() const {  std::lock_guard<std::mutex> lock(_mutex); return updatePeerAddress(); }
	// safe-threading
	SocketAddress&			address(SocketAddress& address) const { std::lock_guard<std::mutex> lock(_mutex); return address.set(updateAddress()); }
	SocketAddress&			peerAddress(SocketAddress& address) const { std::lock_guard<std::mutex> lock(_mutex); return address.set(updatePeerAddress()); }

	bool					connect(Exception& ex, const SocketAddress& address);
	bool					connected() { return _socket.connected(); }
	bool					send(Exception& ex, const UInt8* data, UInt32 size);
	void					disconnect();
	void					close();

	template<typename TCPSenderType>
	bool send(Exception& ex,const std::shared_ptr<TCPSenderType>& pSender) {
		return _socket.send<TCPSenderType>(ex, pSender);
	}
	template<typename TCPSenderType>
	PoolThread*	send(Exception& ex,const std::shared_ptr<TCPSenderType>& pSender, PoolThread* pThread) {
		return _socket.send<TCPSenderType>(ex, pSender, pThread);
	}

	const SocketManager&	manager() const { return _socket.manager(); }

private:
	void closeIntern();

	const SocketAddress&	updateAddress() const { if (_address) return _address;  Exception ex; _socket.address(ex, _address); return _address; }
	const SocketAddress&	updatePeerAddress() const { if (_peerAddress) return _peerAddress; Exception ex; _socket.peerAddress(ex, _peerAddress); return _peerAddress; }

	void						receive(Exception& ex,UInt32 available);
	Socket::OnReadable::Type	onReadable;
	Socket::OnSending::Type		onSending;
	
	Socket					_socket;

	bool					_disconnecting;

	mutable std::mutex		_mutexIdleTime;
	mutable Time			_idleTime;

	mutable std::mutex		_mutex;
	PoolBuffer				_pBuffer;
	mutable SocketAddress	_address;
	mutable SocketAddress	_peerAddress;
	
};


} // namespace Mona
