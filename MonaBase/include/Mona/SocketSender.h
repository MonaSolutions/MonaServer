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

#include "Mona.h"
#include "Mona/WorkThread.h"
#include "Mona/PoolThread.h"
#include "Mona/SocketAddress.h"
#include "Mona/Expirable.h"
#include <memory>


namespace Mona {

class SocketSender : public WorkThread, virtual Object {
	friend class Socket;
public:
	// return true if there is few data available to send
	virtual bool	available() { return begin() && _position < size(); }

	virtual const UInt8*	begin() { return _data; }
	virtual UInt32			size() { return _size; }

protected:
	SocketSender();
	SocketSender(const UInt8* data, UInt32 size);
	virtual ~SocketSender();


	// if return true and ex==true it will display a warning, otherwise return false == failed
	bool							run(Exception& ex);

private:
	// send data
	bool							flush(Exception& ex,Socket& socket);


	//// TO OVERLOAD ////////

	virtual	UInt32					send(Exception& ex,Socket& socket,const UInt8* data, UInt32 size) = 0;

	Expirable<Socket>			_expirableSocket;
	std::weak_ptr<SocketSender>	_pThis;

	UInt32						_position;
	UInt8*						_data;
	UInt32						_size;
	bool						_memcopied;
};


} // namespace Mona
