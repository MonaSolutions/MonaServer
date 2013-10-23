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
#include <memory>


namespace Mona {

class SocketSender : public WorkThread, ObjectFix {
	friend class Socket;
public:
	
protected:
	SocketSender(const UInt8* data, UInt32 size, bool dump = false) :
		_position(0), _data((UInt8*)data), _size(size), _memcopied(false), _dump(dump), _pSocket(NULL) {}
	SocketSender(bool dump = false) : SocketSender(NULL, 0, dump) {}
	virtual ~SocketSender();

	void			dump(bool justInDebug=false);
	
private:
	// send data
	bool			flush(Exception& ex,Socket& socket);

	// return true if there is few data available to send
	virtual bool	available() { return begin() && _position < size(); }

	virtual bool	receiver(std::string& address) { return false; }

	//// TO OVERLOAD ////////

	virtual	UInt32					send(Exception& ex,Socket& socket,const UInt8* data, UInt32 size) = 0;

	virtual const UInt8*			begin(bool dumping = false) { return _data; }
	virtual UInt32					size(bool dumping = false) { return _size; }

	bool							run(Exception& ex);

	std::shared_ptr<std::mutex> _pSocketMutex;
	Socket*						_pSocket;
	std::weak_ptr<SocketSender>	_pThis;

	UInt32						_position;
	UInt8*						_data;
	UInt32						_size;
	bool						_memcopied;
	bool						_dump;
};


} // namespace Mona
