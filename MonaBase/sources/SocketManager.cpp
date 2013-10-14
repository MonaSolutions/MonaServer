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

#include "Mona/SocketManager.h"
#include "Mona/Logs.h"
#include "string.h" // for memset
#if !defined(POCO_OS_FAMILY_WINDOWS)
#include "sys/epoll.h"
#endif

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {


#if defined(POCO_OS_FAMILY_WINDOWS)
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

class SocketManaged : public Poco::RefCountedObject {
public:
	SocketManaged(SocketHandlerBase& handler):handler(handler),socketCopy(*handler.getSocket()) {}
	SocketHandlerBase&			handler;
private:
	Socket						socketCopy;
};	


SocketManager::SocketManager(PoolThreads& poolThreads,UInt32 bufferSize,const string& name) : poolThreads(poolThreads),_eventFD(0),_eventSystem(0),_bufferSize(bufferSize),Startable(name),Task((TaskHandler&)*this),_currentEvent(0),_currentError(0),_event(false),_pCurrentManaged(NULL) {
}

SocketManager::SocketManager(TaskHandler& handler,PoolThreads& poolThreads,UInt32 bufferSize,const string& name) : poolThreads(poolThreads),_eventFD(0),_eventSystem(0),_bufferSize(bufferSize),Startable(name),Task(handler),_currentEvent(0),_currentError(0),_event(false),_pCurrentManaged(NULL) {
}


SocketManager::~SocketManager() {
	stop();
}

void SocketManager::clear() {
	ScopedLock<FastMutex>	lock(_mutex);
	map<poco_socket_t,SocketManaged*>::iterator it;
	for(it=_sockets.begin();it!= _sockets.end();++it) {
		it->second->release();
		if(_eventSystem>0) {
#if defined(POCO_OS_FAMILY_WINDOWS)
			WSAAsyncSelect(it->first,_eventSystem,0,0);
			PostMessage(_eventSystem,0,(WPARAM)it->second,0);
#else
			write(_eventFD,&it->second,sizeof(it->second));
#endif
		}
	}
	_counter = 0;
	_sockets.clear();
}

void SocketManager::start() {
	TaskHandler::start();
	Startable::start();
}

void SocketManager::stop() {
	if(!Startable::running())
		return;
	TaskHandler::stop();
	_event.wait();
	clear();
#if defined(POCO_OS_FAMILY_WINDOWS)
	if(_eventSystem>0)
		PostMessage(_eventSystem,WM_QUIT,0,0);
#else
	if(_eventSystem>0)
		::close(_eventFD);
#endif
	Startable::stop();
	_event.reset();
}


bool SocketManager::open(SocketHandlerBase& handler) const {
	handler._pSocketManaged = NULL;
	if(!Startable::running()) {
		ERROR("Socket manager is not running")
		return false;
	}
	_event.wait();

	if(_eventSystem==0) {
		ERROR("Impossible to manage this socket, the event system hasn't been able to start")
		return false;
	}

	poco_socket_t fd = handler.getSocket()->impl()->sockfd();

	ScopedLock<FastMutex>	lock(_mutex);
	map<poco_socket_t,SocketManaged*>::iterator it = _sockets.lower_bound(fd);
	if(it!=_sockets.end() && it->first==fd) {
		handler._pSocketManaged = it->second;
		return true;
	}

	SocketManaged * pSocketManaged = new SocketManaged(handler);
	pSocketManaged->duplicate();

#if defined(POCO_OS_FAMILY_WINDOWS)
	if(WSAAsyncSelect(fd,_eventSystem,104,FD_ACCEPT | FD_CLOSE | FD_READ)!=0) {
		delete pSocketManaged;
		ERROR("Impossible to manage this socket, code error %d",WSAGetLastError())
		return false;
	}
#else
	epoll_event event;
	 event.events = EPOLLIN | EPOLLRDHUP;
	 event.data.fd = fd;
	 event.data.ptr = pSocketManaged;
	 int res = epoll_ctl(_eventSystem, EPOLL_CTL_ADD,fd, &event);
	if(res<0) {
		try { error(errno);} catch(Exception& ex) {
			delete pSocketManaged;
			ERROR("Impossible to manage this socket, %s",ex.displayText().c_str());
			return false;
		}
	}
#endif

	++_counter;
	if(it!=_sockets.begin())
		--it;
	_sockets.insert(it,pair<poco_socket_t,SocketManaged*>(fd,pSocketManaged));
	handler._pSocketManaged = pSocketManaged;

	if(_bufferSize>0) {
		handler.getSocket()->setReceiveBufferSize(_bufferSize);
		handler.getSocket()->setSendBufferSize(_bufferSize);
	}
	return true;
}

void SocketManager::startWrite(SocketHandlerBase& handler) const {
#if defined(POCO_OS_FAMILY_WINDOWS)
	if(WSAAsyncSelect(handler.getSocket()->impl()->sockfd(),_eventSystem,104,FD_ACCEPT | FD_CLOSE | FD_READ | FD_WRITE)!=0)
		ERROR("Impossible to start writing on this socket, code error %d",WSAGetLastError())
#else
	epoll_event event;
	event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
	event.data.fd = fd;
	event.data.ptr = handler._pSocketManaged;
	int res = epoll_ctl(_eventSystem, EPOLL_CTL_MOD,fd, &event);
	if(res<0) {
		try { error(errno);} catch(Exception& ex) {
			ERROR("Impossible to start writing on this socket, %s",ex.displayText().c_str());
		}
	}
#endif
}

void SocketManager::stopWrite(SocketHandlerBase& handler) const {
#if defined(POCO_OS_FAMILY_WINDOWS)
	if(WSAAsyncSelect(handler.getSocket()->impl()->sockfd(),_eventSystem,104,FD_ACCEPT | FD_CLOSE | FD_READ)!=0)
		WARN("Stop socket writing, code error %d",WSAGetLastError())
#else
	epoll_event event;
	event.events = EPOLLIN | EPOLLRDHUP;
	event.data.fd = fd;
	event.data.ptr = handler._pSocketManaged;
	int res = epoll_ctl(_eventSystem, EPOLL_CTL_MOD,fd, &event);
	if(res<0) {
		try { error(errno);} catch(Exception& ex) {
			WARN("Stop socket writing, %s",ex.displayText().c_str());
		}
	}
#endif
}


void SocketManager::close(SocketHandlerBase& handler) const {
	ScopedLock<FastMutex>	lock(_mutex);
	map<poco_socket_t,SocketManaged*>::iterator it = _sockets.find(handler.getSocket()->impl()->sockfd());
	if(it == _sockets.end())
		return;

	_event.wait();

	it->second->release();

	if(_eventSystem>0) {
#if defined(POCO_OS_FAMILY_WINDOWS)
		WSAAsyncSelect(it->first,_eventSystem,0,0);
		PostMessage(_eventSystem,0,(WPARAM)it->second,0);
#else
		epoll_event event; // Will be ignored by the epoll_ctl call, but is required to work with kernel < 2.6.9
		epoll_ctl(_eventSystem, EPOLL_CTL_DEL, it->first,&event);
		write(_eventFD,&it->second,sizeof(it->second));
#endif
	}

	--_counter;
	_sockets.erase(it);
}

void SocketManager::handle() {
	if(!_error.empty())
		throw Exception(_error);

	if(sockfd()==POCO_INVALID_SOCKET) {
		ERROR("%s manages an invalid socket",name().c_str())
		return;
	}

	SocketManaged* pSocketManaged = _pCurrentManaged;
	if(!pSocketManaged) {
		map<poco_socket_t,SocketManaged*>::const_iterator it = _sockets.find(sockfd());
		if(it==_sockets.end())
			return;
		pSocketManaged = it->second;
	}
	if(pSocketManaged->referenceCount()==1)
		return;
	if(_currentError!=0) {
		try {
			error(_currentError);
		} catch(Exception& ex) {
			pSocketManaged->handler.onError(ex.displayText());
		}
		return;
	}
#if defined(POCO_OS_FAMILY_WINDOWS)
	if(_currentEvent==FD_READ && available()==0) // In the linux case, when _currentEvent==SELECT_READ with 0 bytes it's a ACCEPT event!
		return;
#endif
	try {
		pSocketManaged->handler.onReadable();
	} catch(Exception& ex) {
		pSocketManaged->handler.onError(ex.displayText());
	}
}


void SocketManager::run() {
#if defined(POCO_OS_FAMILY_WINDOWS)
	const char* name = Startable::name().c_str();
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
		_error = "Impossible to create the event system";
#else
	int pipefds[2] = {};
    pipe(pipefds);
    int readFD = pipefds[0];
    _eventFD = pipefds[1];
	if(_eventFD>0)
		_eventSystem = epoll_create(1); // Argument is ignored, but has to be greater or equal to 1
	if(_eventFD<=0 || _eventSystem<=0) {
		_eventSystem = _eventFD = 0;
		try {error(errno);} catch(Exception& ex) {
			_error = format("Impossible to create the event system, %s",ex.displayText());
		}	
	} else {
		// Add the event to terminate the epoll_wait!
		epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = readFD;
		epoll_ctl(_eventSystem, EPOLL_CTL_ADD,readFD, &event);	
	}
#endif

	_event.set();
	if(!_error.empty()) {
		Task::waitHandle();
		return;
	}
	
#if defined(POCO_OS_FAMILY_WINDOWS)
	MSG msg;
    while(GetMessage(&msg,_eventSystem, 0, 0)) {
		if(msg.wParam==0)
			continue;
		if(msg.message==0) {
			SocketManaged* pSocketManaged = (SocketManaged*)msg.wParam;
			pSocketManaged->release();
			continue;
		}
		_currentEvent = WSAGETSELECTEVENT(msg.lParam);
		reset(msg.wParam);
		if(_currentEvent == FD_WRITE) {
			ScopedLock<FastMutex>	lock(_mutex);
			map<poco_socket_t,SocketManaged*>::const_iterator it = _sockets.find(sockfd());
			if(it!=_sockets.end())
				it->second->handler.flushSocket();
		} else if(_currentEvent!=FD_READ || available()) {
			_currentError = WSAGETSELECTERROR(msg.lParam);
			if(_currentError == POCO_ECONNABORTED)
				_currentError = 0;
			Task::waitHandle();
		}
		reset();
	}
	DestroyWindow(_eventSystem);

#else

	int count = _counter.value()+1;
	vector<epoll_event> events(count);

	for(;;) {

		int results = epoll_wait(_eventSystem,&events[0],events.size(), -1);

		if(results<0 && errno!=EINTR) {
			try {error(errno);} catch(Exception& ex) {
				_error = format("Socket manager error, %s",ex.displayText());
				waitHandle();
				break;
			}
		}

		// for each ready socket
		int i=0;
		for(i;i<results;++i) {
			epoll_event event = events[i];
			if(event.data.fd==readFD) {
				SocketManaged* pSocketManaged(NULL);
				read(readFD,&pSocketManaged,sizeof(pSocketManaged));
				if(!pSocketManaged) {
					i=-1; // termination signal!
					break;
				}
				pSocketManaged->release();
				continue;		
			}
			_pCurrentManaged = (SocketManaged*)event.data.ptr;
			if(!_pCurrentManaged)
				continue;
			reset(_pCurrentManaged->socketCopy.impl()->sockfd());
			_currentEvent = event.events;
			if(_pCurrentManaged->referenceCount()>1) {
				if(_currentEvent&EPOLLERR)
					_currentError = lastError();
				if(_currentError==0 && _currentEvent&EPOLLOUT)
					_pCurrentManaged->handler.flushSocket();
				else
					waitHandle();
			}
			_currentError = 0;
			_pCurrentManaged = NULL;
			reset();
		}
		if(i==-1)
			break; // termination signal!
		count = _counter.value()+1;
		if(count!=events.size())
			events.resize(count);
	}

	::close(_eventSystem);
#endif

}

	


} // namespace Mona
