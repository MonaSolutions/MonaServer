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
#include "Mona/SocketHandler.h"
#include "Mona/PoolThread.h"
#include "Poco/Net/SocketAddress.h"

namespace Mona {

class SocketSender : public WorkThread {
public:
	void			release();
	virtual bool	flush();
	virtual bool	available();

	PoolThread*		go(PoolThread* pThread);

protected:
	SocketSender(SocketHandlerBase& handler,const Mona::UInt8* data,Mona::UInt32 size,bool dump=false);
	SocketSender(SocketHandlerBase& handler,bool dump=false);
	virtual ~SocketSender();

	void									dump(bool justInDebug=false);
	
private:
	virtual	Mona::UInt32					send(const Mona::UInt8* data,Mona::UInt32 size)=0;
	virtual const Poco::Net::SocketAddress&	receiver()=0;
	virtual void							pack(){}

	virtual const Mona::UInt8*				begin(bool dumping=false);
	virtual Mona::UInt32					size(bool dumping=false);

	void									run();

	Poco::SharedPtr<bool>		_pSocketClosed;
	Mona::UInt32				_position;
	Mona::UInt8*				_data;
	Mona::UInt32				_size;
	bool						_memcopied;
	bool						_dump;
	SocketHandlerBase&			_handler;
	bool						_wrote;
	volatile bool				_running;
	Poco::FastMutex				_mutex;
};

inline bool SocketSender::available() {
	return _handler.getSocket() && begin() && _position < size();
}

inline const Mona::UInt8* SocketSender::begin(bool dumping) {
	return _data;
}

inline Mona::UInt32 SocketSender::size(bool dumping) {
	return _size;
}


} // namespace Mona
