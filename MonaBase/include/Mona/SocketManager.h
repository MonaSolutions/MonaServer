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
#include "Mona/Startable.h"
#include "Mona/TaskHandler.h"
#include "Mona/PoolThreads.h"
#include "Mona/Socket.h"
#include <map>

namespace Mona {

class SocketManager : private Task, private Startable, private TaskHandler, virtual Object {
	friend class Socket;
public:
	SocketManager(TaskHandler& handler, PoolThreads& poolThreads, UInt32 bufferSize = 0, const std::string& name = "SocketManager");
	SocketManager(PoolThreads& poolThreads, UInt32 bufferSize = 0, const std::string& name = "SocketManager");
	virtual ~SocketManager() { stop(); }

	void					start(Exception& ex);
	void					stop();

private:
	class FakeSocket : public Socket {
	public:
		FakeSocket(SocketManager& manager) : Socket(manager) {}
	private:
		virtual void	onReadable(Exception& ex) {}
		virtual void	onError(const std::string& error) {}
	};
	
	// add a socket with a valid file descriptor to manage it
	bool add(Exception& ex,Socket& socket) const;

	// remove a socket with a valid file descriptor to unmanage it
	void remove(Socket& socket) const;

	bool startWrite(Exception& ex, Socket& socket) const;
	bool stopWrite(Exception& ex, Socket& socket) const;

	void					requestHandle();
	void					clear();
	void					run(Exception& ex);
	void					handle(Exception& ex);

	bool								_selfHandler;
	Exception							_ex;
	UInt32								_bufferSize;
	PoolThreads&						_poolThreads;

	mutable std::atomic<int>			_counter;
	mutable Event						_eventInit;
	mutable std::mutex					_mutex;

	mutable std::map<SOCKET, std::unique_ptr<Socket>*>		_sockets;

#if defined(_WIN32)
	HWND								_eventSystem;
	FakeSocket							_fakeSocket;
	Exception							_exSkip;
#else
	int									_eventSystem;
#endif
	int									_eventFD; // used just in linux case
	
	UInt32								_currentEvent;
	int									_currentError;

	std::unique_ptr<Socket>*			_ppSocket;
	SOCKET								_sockfd;
};


} // namespace Mona
