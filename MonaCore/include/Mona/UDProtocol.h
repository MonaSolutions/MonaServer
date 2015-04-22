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
#include "Mona/Protocol.h"
#include "Mona/UDPSocket.h"
#include "Mona/Logs.h"

namespace Mona {

class UDProtocol : public Protocol, public virtual Object,
	public Events::OnPacket,
	public Events::OnError {
public:
	bool load(Exception& ex, const SocketAddress& address) {
		return _socket.bind(ex, address);
	}

	template<typename ProtocolType,typename UDPSenderType>
	bool send(Exception& ex,const std::shared_ptr<UDPSenderType>& pSender) {
		return _socket.send<UDPSenderType>(ex, pSender);
	}
	template<typename ProtocolType,typename UDPSenderType>
	PoolThread*	send(Exception& ex,const std::shared_ptr<UDPSenderType>& pSender, PoolThread* pThread) {
		return _socket.send<UDPSenderType>(ex, pSender, pThread);
	}
	

protected:
	UDProtocol(const char* name, Invoker& invoker, Sessions& sessions) : _socket(invoker.sockets), Protocol(name, invoker, sessions) {
		onPacket = [this](PoolBuffer& pBuffer, const SocketAddress& address) {
			if(!auth(address))
				return;
			OnPacket::raise(pBuffer,address);
		};
		if (!OnError::subscribed()) {
			onError = [this](const Exception& ex) { DEBUG("Protocol ", this->name, ", ", ex.error()); };
			_socket.OnError::subscribe(onError);
		} else
			_socket.OnError::subscribe(*this);
		_socket.OnPacket::subscribe(onPacket);
	}
	virtual ~UDProtocol() {
		if (onError)
			_socket.OnError::unsubscribe(onError);
		else
			_socket.OnError::unsubscribe(*this);
		_socket.OnPacket::unsubscribe(onPacket);
	}

private:
	UDPSocket::OnError::Type	onError;
	UDPSocket::OnPacket::Type	onPacket;

	UDPSocket	_socket;
};


} // namespace Mona
