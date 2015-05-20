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

#include "Mona.h"
#include "Mona/SocketAddress.h"
#include "Mona/WorkThread.h"
#include "Mona/PoolThread.h"
#include "Mona/PoolBuffer.h"
#include <memory>


namespace Mona {

class Socket;
class SocketSender : public WorkThread, public Binary, public virtual Object {
	friend class Socket;
	friend class SocketImpl;
public:
	bool	available() { return _ppBuffer ? !_ppBuffer->empty() : (data() && _position < size()); }

	virtual const UInt8*	data() const { return _data; }
	virtual UInt32			size() const { return _size; }

protected:
	SocketSender(const char* name);
	SocketSender(const char* name,const UInt8* data, UInt32 size);


	// if return true and ex==true it will display a warning, otherwise return false == failed
	bool							run(Exception& ex);

private:
	
	bool							buffering(const PoolBuffers& poolBuffers);

	// send data, return true if everything has been sent
	bool							flush(Exception& ex,Socket& socket);


	//// TO OVERLOAD ////////

	virtual	UInt32					send(Exception& ex,Socket& socket,const UInt8* data, UInt32 size) = 0;
	virtual void					onSent(Socket& socket) {}

	std::unique_ptr<Socket>		_pSocket;
	std::weak_ptr<SocketSender>	_pThis;

	UInt32						_position;
	UInt8*						_data;
	UInt32						_size;
	std::unique_ptr<PoolBuffer>	_ppBuffer;
};


} // namespace Mona
