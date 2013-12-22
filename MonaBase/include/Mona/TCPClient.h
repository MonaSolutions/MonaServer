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
#include "Mona/StreamSocket.h"
#include "Mona/PoolBuffer.h"

namespace Mona {


class TCPClient : protected StreamSocket, virtual Object {
public:
	TCPClient(const SocketManager& manager);
	TCPClient(const SocketAddress& peerAddress,const SocketManager& manager);
	virtual ~TCPClient();

	bool					connect(Exception& ex, const SocketAddress& address);
	bool					connected() { return _connected; }
	bool					send(Exception& ex, const UInt8* data, UInt32 size);

	template<typename SenderType>
	void send(Exception& ex, std::shared_ptr<SenderType>& pSender) {
		StreamSocket::send<SenderType>(ex, pSender);
	}
	template<typename SenderType>
	PoolThread*	send(Exception& ex, std::shared_ptr<SenderType>& pSender, PoolThread* pThread) {
		return StreamSocket::send<SenderType>(ex, pSender, pThread);
	}

	void					disconnect();

	const SocketAddress&	address();
	const SocketAddress&	peerAddress();

private:
	virtual UInt32			onReception(const UInt8* data,UInt32 size) = 0;
	virtual void			onError(const std::string& error) = 0;
	virtual void			onDisconnection() {}


	void					onReadable(Exception& ex);
	
	int						sendIntern(const UInt8* data,UInt32 size);

	PoolBuffer				_pBuffer;
	UInt32					_rest;
	bool					_connected;

	SocketAddress			_address;
	SocketAddress			_peerAddress;
};


} // namespace Mona
