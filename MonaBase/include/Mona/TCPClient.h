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
#include "Mona/SocketHandler.h"
#include "Mona/PoolBuffer.h"

namespace Mona {


class TCPClient : public SocketHandler, virtual Object {
public:
	TCPClient(const SocketManager& manager);
	TCPClient(const SocketAddress& peerAddress,const SocketManager& manager);
	virtual ~TCPClient();

	const SocketAddress&	address() { return SocketHandler::address(); }
	const SocketAddress&	peerAddress() { return SocketHandler::peerAddress(); }

	bool					connect(Exception& ex, const SocketAddress& address);
	bool					connected() { return _connected; }
	bool					send(Exception& ex, const UInt8* data, UInt32 size);
	void					disconnect();

	template<typename SenderType>
	bool send(Exception& ex,const std::shared_ptr<SenderType>& pSender) {
		return socket().send<SenderType>(ex, pSender);
	}
	template<typename SenderType>
	PoolThread*	send(Exception& ex,const std::shared_ptr<SenderType>& pSender, PoolThread* pThread) {
		return SocketHandler::send<SenderType>(ex, pSender, pThread);
	}

private:
	
	virtual UInt32			onReception(PoolBuffer& pBuffer) = 0;
	virtual void			onDisconnection() {}


	void					onReadable(Exception& ex);

	volatile bool			_connected;
	std::recursive_mutex	_mutex;
	PoolBuffer				_pBuffer;
	UInt32					_rest;
	
};


} // namespace Mona
