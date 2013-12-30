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
#include "Mona/Startable.h"
#include "Mona/TaskHandler.h"
#include "Mona/PoolThreads.h"
#include "Mona/PoolBuffers.h"
#include "Mona/Socket.h"
#include <map>
#include <atomic>

namespace Mona {

class SocketManager : private Task, private Startable, private TaskHandler, virtual Object {
	friend class Socket;
public:
	SocketManager(TaskHandler& handler, PoolBuffers& poolBuffers, PoolThreads& poolThreads, UInt32 bufferSize = 0, const std::string& name = "SocketManager");
	SocketManager(PoolBuffers& poolBuffers, PoolThreads& poolThreads, UInt32 bufferSize = 0, const std::string& name = "SocketManager");
	virtual ~SocketManager() { stop(); }

	bool					start(Exception& ex);
	void					stop();

	PoolThreads&			poolThreads;
	PoolBuffers&			poolBuffers;

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

	mutable std::atomic<int>			_counter;
	mutable Event						_eventInit;
	mutable std::mutex					_mutex;

    mutable std::map<NET_SOCKET, std::unique_ptr<Socket>*>		_sockets;

    FakeSocket							_fakeSocket;
    Exception							_exSkip;
#if defined(_WIN32)
    HWND								_eventSystem;
#else
	int									_eventSystem;
#endif
	int									_eventFD; // used just in linux case
	
	UInt32								_currentEvent;
	int									_currentError;

	std::unique_ptr<Socket>*			_ppSocket;
    NET_SOCKET							_sockfd;
};


} // namespace Mona
