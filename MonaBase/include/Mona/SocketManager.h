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
#include "Mona/Exceptions.h"
#include "Mona/Startable.h"
#include "Mona/TaskHandler.h"
#include "Mona/PoolThreads.h"
#include "Mona/SocketHandler.h"
#include <map>

namespace Mona {

class SocketManaged;
class SocketManager : private Poco::Net::SocketImpl, private Task, private Startable, private TaskHandler {
	friend class SocketHandlerBase;
public:
	SocketManager(TaskHandler& handler,PoolThreads& poolThreads,Mona::UInt32 bufferSize=0,const std::string& name="SocketManager");
	SocketManager(PoolThreads& poolThreads,Mona::UInt32 bufferSize=0,const std::string& name="SocketManager");
	virtual ~SocketManager();

	PoolThreads&			poolThreads;

	void					start();
	void					stop();

private:
	bool open(SocketHandlerBase& handler) const;
	void close(SocketHandlerBase& handler) const;
	void startWrite(SocketHandlerBase& handler) const;
	void stopWrite(SocketHandlerBase& handler) const;

	void					requestHandle();
	void					clear();
	void					run();
	void					handle();

	Mona::UInt32									_bufferSize;
	std::string										_error;

	mutable std::map<poco_socket_t,SocketManaged*>	_sockets;
	mutable Poco::AtomicCounter						_counter;
	mutable Poco::Event								_event;
	mutable Poco::FastMutex							_mutex;

#if defined(POCO_OS_FAMILY_WINDOWS)
	HWND										_eventSystem;
#else
	int											_eventSystem;
#endif

	int											_eventFD; // used just in linux case
	SocketManaged*								_pCurrentManaged;
	Mona::UInt32								_currentEvent;
	int											_currentError;
};

inline void SocketManager::requestHandle() {
	giveHandle();
};


} // namespace Mona
