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
#include "Mona/DatagramSocket.h"
#include "Mona/PoolBuffer.h"

namespace Mona {

class UDPSocket : protected DatagramSocket, virtual Object {
public:
	UDPSocket(const SocketManager& manager,bool allowBroadcast=false);

	bool					bind(Exception& ex, const SocketAddress& address);
	bool					connect(Exception& ex, const SocketAddress& address);
	void					close();

	bool					send(Exception& ex, const UInt8* data, UInt32 size);
	bool					send(Exception& ex, const UInt8* data, UInt32 size, const SocketAddress& address);

	template<typename SenderType>
	void send(Exception& ex, std::shared_ptr<SenderType>& pSender) {
		DatagramSocket::send<SenderType>(ex, pSender);
	}
	template<typename SenderType>
	PoolThread*	send(Exception& ex, std::shared_ptr<SenderType>& pSender, PoolThread* pThread) {
		return DatagramSocket::send<SenderType>(ex, pSender, pThread);
	}

	const SocketAddress&	address();
	const SocketAddress&	peerAddress();

	Socket&					socket() { return *this; }

protected:
	PoolBuffer&				rawBuffer() { return _pBuffer; }

private:
	virtual void			onReception(const UInt8* data, UInt32 size, const SocketAddress& address) = 0;
	void					onReadable(Exception& ex);

	bool					_allowBroadcast;
	bool					_broadcasting;

	SocketAddress			_address;
	SocketAddress			_peerAddress;
	PoolBuffer				_pBuffer;
};



} // namespace Mona
