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

#include "Mona/SocketManager.h"
#if !defined(_WIN32)
#include <unistd.h>
#include "sys/epoll.h"
#endif

using namespace std;

namespace Mona {


#if defined(_WIN32)
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif


SocketManager::SocketManager(TaskHandler& handler, const PoolBuffers& poolBuffers, PoolThreads& poolThreads, UInt32 bufferSize, const string& name) : poolBuffers(poolBuffers),
    _fakeSocket(*this), _selfHandler(false), poolThreads(poolThreads), _eventFD(0), _sockfd(NET_INVALID_SOCKET), _eventSystem(0), bufferSize(bufferSize), Startable(name), Task(handler), _currentEvent(0), _currentError(0), _eventInit(false), _ppSocket(NULL),_counter(0) {
	_fakeSocket._initialized = true;
}
SocketManager::SocketManager(const PoolBuffers& poolBuffers, PoolThreads& poolThreads, UInt32 bufferSize, const string& name) : poolBuffers(poolBuffers),
    _fakeSocket(*this), _selfHandler(true), poolThreads(poolThreads), _eventFD(0), _sockfd(NET_INVALID_SOCKET), _eventSystem(0), bufferSize(bufferSize), Startable(name), Task((TaskHandler&)*this), _currentEvent(0), _currentError(0), _eventInit(false), _ppSocket(NULL),_counter(0) {
	_fakeSocket._initialized = true;
}

bool SocketManager::start(Exception& ex) {
	TaskHandler::start();
	return Startable::start(ex);
}

void SocketManager::stop() {
	if (!Startable::running())
		return;
	TaskHandler::stop();
	_eventInit.wait();

	{
		// clear sockets
		lock_guard<recursive_mutex> lock(_mutex);
		
		for (auto& it : _sockets) {
			it.second->release(); // don't delete the pSocket!

#if defined(_WIN32)
			if (_eventSystem == 0 || (WSAAsyncSelect(it.first, _eventSystem, 0, 0) == 0 && !PostMessage(_eventSystem, 0, (WPARAM)it.second, 0))) {
#else
			epoll_event event;
			memset(&event, 0, sizeof(event));
			if (_eventSystem==0 || (epoll_ctl(_eventSystem, EPOLL_CTL_DEL, it.first,&event)>=0 && write(_eventFD,&it.second,sizeof(it.second))<0)) {
#endif
				delete it.second;
			}
		}
		_counter = 0;
		_sockets.clear();
	}

#if defined(_WIN32)
	if (_eventSystem > 0)
		PostMessage(_eventSystem, WM_QUIT, 0, 0);
#else
	if (_eventSystem > 0)
		::close(_eventFD);
#endif
	Startable::stop();
	_eventInit.reset();
}



bool SocketManager::add(Exception& ex,Socket& socket) const {
	if (!Startable::running()) {
		ex.set(Exception::SOCKET, name(), "is not running");
		return false;
	}

	_eventInit.wait();

	if(_eventSystem==0) {
		ex.set(Exception::SOCKET, name(), " hasn't been able to start, impossible to manage sockets");
		return false;
	}

    NET_SOCKET sockfd = socket._sockfd;


	lock_guard<recursive_mutex> lock(_mutex);

	auto it = _sockets.lower_bound(sockfd);
	if (it != _sockets.end() && it->first == sockfd)
		return true; // already managed
	
	
	unique_ptr<Socket>* ppSocket = new unique_ptr<Socket>(&socket);

#if defined(_WIN32)
	int flags = FD_ACCEPT | FD_CLOSE | FD_READ;
	if (WSAAsyncSelect(sockfd, _eventSystem, 104, flags) != 0) {
		ppSocket->release();
		delete ppSocket;
		Net::SetError(ex);
		return false;
	}
#else
	epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN | EPOLLRDHUP;
	 event.data.fd = sockfd;
	 event.data.ptr = ppSocket;
	 int res = epoll_ctl(_eventSystem, EPOLL_CTL_ADD,sockfd, &event);
	if(res<0) {
		ppSocket->release();
		delete ppSocket;
        Net::SetError(ex);
		return false;
	}
#endif

	++_counter;
	socket._ppSocket = ppSocket;
	_sockets.emplace_hint(it, sockfd, ppSocket);

	if(bufferSize>0) {
		socket.setReceiveBufferSize(ex, bufferSize);
		socket.setSendBufferSize(ex, bufferSize);
	}

	return true;
}

bool SocketManager::startWrite(Socket& socket) const {
#if defined(_WIN32)
	return WSAAsyncSelect(socket._sockfd, _eventSystem, 104, FD_ACCEPT | FD_CLOSE | FD_READ | FD_WRITE) == 0;
#else
	epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
	event.data.fd = socket._sockfd;
	event.data.ptr = socket._ppSocket;
    return epoll_ctl(_eventSystem, EPOLL_CTL_MOD, socket._sockfd, &event)>=0;
#endif
}

bool SocketManager::stopWrite(Socket& socket) const {
#if defined(_WIN32)
	return WSAAsyncSelect(socket._sockfd, _eventSystem, 104, FD_ACCEPT | FD_CLOSE | FD_READ) == 0;
#else
	epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN | EPOLLRDHUP;
	event.data.fd = socket._sockfd;
	event.data.ptr = socket._ppSocket;
    return epoll_ctl(_eventSystem, EPOLL_CTL_MOD, socket._sockfd, &event) >= 0;
#endif
}


void SocketManager::remove(Socket& socket) const {
	lock_guard<recursive_mutex>	lock(_mutex);
	auto it = _sockets.find(socket._sockfd);
	if(it == _sockets.end())
		return;

	_eventInit.wait();

	--_counter;
	socket._ppSocket->release();
	_sockets.erase(it);

#if defined(_WIN32)
	if (_eventSystem==0 || (WSAAsyncSelect(socket._sockfd,_eventSystem,0,0)==0 && !PostMessage(_eventSystem,0,(WPARAM)socket._ppSocket,0)))
		delete socket._ppSocket;
#else
	epoll_event event;
	memset(&event, 0, sizeof(event));
	if (_eventSystem==0 || (epoll_ctl(_eventSystem, EPOLL_CTL_DEL,socket._sockfd,&event)>=0 && write(_eventFD,&socket._ppSocket,sizeof(socket._ppSocket))<0))
		delete socket._ppSocket;
#endif
}

void SocketManager::handle(Exception& ex) {
	if (_ex) {
		ex.set(_ex);
		return;
	}

	lock_guard<recursive_mutex>	lock(_mutex);

	Socket* pSocket(NULL);
	if (_ppSocket)
		pSocket = _ppSocket->get();
	if (!pSocket) {
		auto it = _sockets.find(_sockfd);
		if(it==_sockets.end())
			return;
		pSocket = it->second->get();
	}

	if(_currentError!=0) {
		Exception curEx;
		Net::SetError(curEx, _currentError);
		pSocket->onError(curEx.error());
	}

	if (_currentEvent == 0)
		return;

	/// now, read, accept, or hangup event

#if defined(_WIN32)
	if(_currentEvent==FD_READ && pSocket->available(_exSkip)==0) // In the linux case, when _currentEvent==SELECT_READ with 0 bytes it's a ACCEPT event!
		return;
#else
	do { // call onReadable at minimum one time (it can be an accept or hangup)
#endif
		Exception socketEx;
		pSocket->onReadable(socketEx);
		if (socketEx)
			pSocket->onError(socketEx.error());
#if !defined(_WIN32)
	} while (pSocket->available(_exSkip) > 0);  // just one time for windows, and fill available bytes for posix
#endif
}

void SocketManager::requestHandle() {
	Exception ex;
	giveHandle(ex);
}

void SocketManager::run(Exception& exc) {
	Exception& ex(_selfHandler ? exc : _ex);
	const char* name = Startable::name().c_str();
	_eventSystem = 0;
#if defined(_WIN32)
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = name;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	RegisterClassEx(&wc);
	_eventSystem = CreateWindow(name, name, WS_EX_LEFT, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
	if(_eventSystem==0)
		ex.set(Exception::NETWORK, name, " starting failed, impossible to manage sockets");
#else
	int pipefds[2] = {};
	int readFD(0);
	_eventFD=0;
	if(pipe(pipefds)==0) {
		readFD = pipefds[0];
		_eventFD = pipefds[1];
	}
	if(readFD>0 && _eventFD>0)
		_eventSystem = epoll_create(1); // Argument is ignored, but has to be greater or equal to 1
	if(_eventSystem<=0) {
		if(_eventFD>0)
			::close(_eventFD);
		if(readFD>0)
			::close(readFD);
		_eventSystem = 0;
		ex.set(Exception::NETWORK, name, " starting failed, impossible to manage sockets");
	} else {
		// Add the event to terminate the epoll_wait!
		epoll_event event;
		memset(&event, 0, sizeof(event));
		event.events = EPOLLIN;
		event.data.fd = readFD;
		epoll_ctl(_eventSystem, EPOLL_CTL_ADD,readFD, &event);	
	}
#endif

	_eventInit.set();
	if (ex) { // here _eventSystem==0
		Task::waitHandle();
		return;
	}



#if defined(_WIN32)
	MSG		  msg;
	int result;
    while((result=GetMessage(&msg,_eventSystem, 0, 0))>0) {
		if(msg.wParam==0)
			continue;
		if(msg.message==0) {
			unique_ptr<Socket>* ppSocket = (unique_ptr<Socket>*)msg.wParam;
			// no mutex for ppSocket methods access because the socket has been removed already here!
			delete ppSocket;
			continue;
		} else if (msg.message != 104) // unknown message
			continue;
		_currentEvent = WSAGETSELECTEVENT(msg.lParam);
		_fakeSocket._sockfd = _sockfd = msg.wParam;
		if(_currentEvent == FD_WRITE) {

			// protected for _sockets access
			lock_guard<recursive_mutex> lock(_mutex);
			auto& it = _sockets.find(_sockfd);
			if (it != _sockets.end()) {
				Socket* pSocket = (*it->second).get();
				Exception curEx;
				pSocket->flushSenders(curEx);
				if (curEx)
					pSocket->onError(curEx.error());
			}

		} else if (_currentEvent != FD_READ || _fakeSocket.available(_exSkip)) {
			if (_currentEvent!=FD_CLOSE)
				_currentError = WSAGETSELECTERROR(msg.lParam);
			Task::waitHandle();EPOLL_CTL_DEL
		}

		_sockfd = INVALID_SOCKET;
	}
	DestroyWindow(_eventSystem);
	if (result<0)
		ex.set(Exception::NETWORK,name, " failed, impossible to manage sockets");
#else
	_fakeSocket._sockfd = readFD;
    int count = _counter+1;
	vector<epoll_event> events(count);
	vector<unique_ptr<Socket>*>	removedSockets;

	for(;;) {

		int results = epoll_wait(_eventSystem,&events[0],events.size(), -1);

		if(results<0 && errno!=EINTR) {
			Net::SetError(ex);
			break;
		}

		// for each ready socket
		int i=0;
		for(i;i<results;++i) {
			epoll_event& event = events[i];

			if(event.data.fd==readFD) {

				if(event.events&EPOLLHUP) {
					i=-1; // termination signal!
					break;
				}

				unique_ptr<Socket>* ppSocket(NULL);
				while (_fakeSocket.available(_exSkip)>=sizeof(ppSocket) && read(readFD, &ppSocket, sizeof(ppSocket)) > 0) {
					// no mutex for ppSocket methods access because the socket has been removed already here!
					removedSockets.push_back(ppSocket);
				}
				continue;	
			}
		
			_currentEvent = event.events;
			if(_currentEvent&EPOLLERR) {
                _currentError = Net::LastError();
				_currentEvent &= ~EPOLLERR;
			}

			if(_currentEvent&EPOLLOUT) {
				_ppSocket = (unique_ptr<Socket>*)event.data.ptr;
				// protected for _ppSocket methods access
				lock_guard<recursive_mutex> lock(_mutex);
				Socket* pSocket(_ppSocket->get());
				if(pSocket) {
					Exception curEx;
					pSocket->flushSenders(curEx);
					if (curEx)
						pSocket->onError(curEx.error());
				}
				_currentEvent &= ~EPOLLOUT;
			}

			if(_currentError>0 || _currentEvent>0) {
				_ppSocket = (unique_ptr<Socket>*)event.data.ptr;
				Task::waitHandle();
			}

			_currentError = 0;
			_ppSocket = NULL;
		}
		if(i==-1)
			break; // termination signal!
        count = _counter+1;
		if(count!=events.size())
			events.resize(count);

		// remove sockets
		if(removedSockets.empty())
			continue;
		for (unique_ptr<Socket>* ppSocket : removedSockets)
			delete ppSocket;
		removedSockets.clear();
	}
	::close(readFD);  // close reader pipe side
	::close(_eventSystem); // close the system message

	for (unique_ptr<Socket>* ppSocket : removedSockets)
		delete ppSocket;
	removedSockets.clear();
#endif

	if (ex)
		Task::waitHandle();
}

	


} // namespace Mona
