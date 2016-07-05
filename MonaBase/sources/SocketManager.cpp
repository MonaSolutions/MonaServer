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
#include "Mona/Socket.h"
#if defined(_OS_BSD)
    #include <sys/types.h>
    #include <sys/event.h>
    #include <sys/time.h>
    #include <vector>
#elif !defined(_WIN32)
    #include <unistd.h>
    #include "sys/epoll.h"
    #include <vector>
#endif

using namespace std;

namespace Mona {


#if defined(_WIN32)
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif


SocketManager::SocketManager(TaskHandler& handler, const PoolBuffers& poolBuffers, PoolThreads& poolThreads, UInt32 bufferSize, const string& name) : poolBuffers(poolBuffers),
   _selfHandler(false), poolThreads(poolThreads), _eventFD(0), _sockfd(NET_INVALID_SOCKET), _eventSystem(0), bufferSize(bufferSize), Startable(name), Task(handler), _currentEvent(0), _currentError(0), _initSignal(false), _ppSocket(NULL),_counter(0) {

}
SocketManager::SocketManager(const PoolBuffers& poolBuffers, PoolThreads& poolThreads, UInt32 bufferSize, const string& name) : poolBuffers(poolBuffers),
   _selfHandler(true), poolThreads(poolThreads), _eventFD(0), _sockfd(NET_INVALID_SOCKET), _eventSystem(0), bufferSize(bufferSize), Startable(name), Task((TaskHandler&)*this), _currentEvent(0), _currentError(0), _initSignal(false), _ppSocket(NULL),_counter(0) {

}

bool SocketManager::start(Exception& ex) {
	if (Startable::running())
		return true;
	_initSignal.reset();
	TaskHandler::start();
	return Startable::start(ex);
}

void SocketManager::stop() {
	if (!Startable::running())
		return;
	TaskHandler::stop();
	_initSignal.wait();

#if defined(_WIN32)
	if (_eventSystem > 0)
		PostMessage(_eventSystem, WM_QUIT, 0, 0);
#else
	if (_eventSystem > 0)
		::close(_eventFD);
#endif
	Startable::stop();
}



Socket** SocketManager::add(Exception& ex,NET_SOCKET sockfd,Socket& socket) const {
	if (!Startable::running()) {
		ex.set(Exception::SOCKET, name(), "is not running");
		return NULL;
	}

	_initSignal.wait();

	if(_eventSystem==0) {
		ex.set(Exception::SOCKET, name(), " hasn't been able to start, impossible to manage sockets");
		return NULL;
	}

	lock_guard<recursive_mutex> lock(_mutex);

	auto it = _sockets.lower_bound(sockfd);
	if (it != _sockets.end() && it->first == sockfd)
		return it->second; // already managed
	
	Socket** ppSocket = new Socket*(&socket);

#if defined(_WIN32)
	int flags = FD_CONNECT | FD_ACCEPT | FD_CLOSE | FD_READ;
	if (WSAAsyncSelect(sockfd, _eventSystem, 104, flags) != 0) {
		delete ppSocket;
		Net::SetException(ex, Net::LastError());
		return NULL;
	}
#elif defined(_OS_BSD)
	struct kevent event;
	memset(&event, 0, sizeof(event));
	EV_SET(&event, sockfd, EVFILT_READ, EV_ADD, 0, 0, ppSocket);
	int res = kevent(_eventSystem, &event, 1, NULL, 0, NULL);
	if(res<0) {
		delete ppSocket;
		Net::SetException(ex, Net::LastError());
		return NULL;
	}
#else
	epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN | EPOLLRDHUP;
	 event.data.fd = sockfd;
	 event.data.ptr = ppSocket;
	 int res = epoll_ctl(_eventSystem, EPOLL_CTL_ADD,sockfd, &event);
	if(res<0) {
		delete ppSocket;
		Net::SetException(ex, Net::LastError());
		return NULL;
	}
#endif

	++_counter;
	_sockets.emplace_hint(it, sockfd, ppSocket);

	if(bufferSize>0) {
		socket.setReceiveBufferSize(ex, bufferSize);
		socket.setSendBufferSize(ex, bufferSize);
	}

	return ppSocket;
}

bool SocketManager::startWrite(NET_SOCKET sockfd,Socket** ppSocket) const {
#if defined(_WIN32)
	return WSAAsyncSelect(sockfd, _eventSystem, 104, FD_CONNECT | FD_ACCEPT | FD_CLOSE | FD_READ | FD_WRITE) == 0;
#elif defined(_OS_BSD)
	struct kevent event;
	memset(&event, 0, sizeof(event));
	EV_SET(&event, sockfd, EVFILT_WRITE, EV_ADD, 0, 0, ppSocket);
	return kevent(_eventSystem, &event, 1, NULL, 0, NULL) >= 0;
#else
	epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
	event.data.fd = sockfd;
	event.data.ptr = ppSocket;
    return epoll_ctl(_eventSystem, EPOLL_CTL_MOD, sockfd, &event)>=0;
#endif
}

bool SocketManager::stopWrite(NET_SOCKET sockfd,Socket** ppSocket) const {
#if defined(_WIN32)
	return WSAAsyncSelect(sockfd, _eventSystem, 104, FD_CONNECT | FD_ACCEPT | FD_CLOSE | FD_READ) == 0;
#elif defined(_OS_BSD)
	struct kevent event;
	memset(&event, 0, sizeof(event));
	EV_SET(&event, sockfd, EVFILT_READ, EV_ADD, 0, 0, ppSocket);
	return kevent(_eventSystem, &event, 1, NULL, 0, NULL) >= 0;
#else
	epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = EPOLLIN | EPOLLRDHUP;
	event.data.fd = sockfd;
	event.data.ptr = ppSocket;
    return epoll_ctl(_eventSystem, EPOLL_CTL_MOD,sockfd, &event) >= 0;
#endif
}


void SocketManager::remove(NET_SOCKET sockfd) const {
	if (!Startable::running())
		return;

	_initSignal.wait();

	if (_eventSystem == 0) // keep it to avoid dead lock on sockets deletion! (and _sockets is necessary empty!)
		return;

	lock_guard<recursive_mutex> lock(_mutex);

	auto it = _sockets.find(sockfd);
	if(it == _sockets.end())
		return;

	Socket** ppSocket = it->second;
	_sockets.erase(it);
	(*ppSocket) = NULL;

#if defined(_WIN32)
	if (_eventSystem==0 || (WSAAsyncSelect(sockfd,_eventSystem,0,0)==0 && !PostMessage(_eventSystem,0,(WPARAM)ppSocket,0)))
		delete ppSocket;
#elif defined(_OS_BSD)
    struct kevent event;
    memset(&event, 0, sizeof(event));
    if (_eventSystem==0)
        delete ppSocket;
    else {
        EV_SET(&event, sockfd, EVFILT_READ | EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
        if (kevent(_eventSystem, &event, 1, NULL, 0, NULL)>=0 && write(_eventFD,&ppSocket,sizeof(ppSocket))<0)
            delete ppSocket;
    }
#else
	epoll_event event;
	memset(&event, 0, sizeof(event));
	if (_eventSystem==0 || (epoll_ctl(_eventSystem, EPOLL_CTL_DEL,sockfd,&event)>=0 && write(_eventFD,&ppSocket,sizeof(ppSocket))<0))
		delete ppSocket;
#endif
	--_counter;
}

void SocketManager::handle(Exception& ex) {
	if (_eventSystem==0) {
		if (_ex)
			ex = _ex;
		lock_guard<recursive_mutex> lock(_mutex);
		if (_sockets.empty())
			return;
		_currentException.set(Exception::NETWORK, "SocketManager is stopping");
		for (auto& it : _sockets) {
			(*it.second)->onError(_currentException);
			delete it.second;
		}
		_sockets.clear();
		return;
	}

	lock_guard<recursive_mutex>	lock(_mutex);

	if (!_ppSocket) {
		if (_sockfd == NET_INVALID_SOCKET)
			return;
		auto it = _sockets.find(_sockfd);
		if(it==_sockets.end())
			return;
		_ppSocket = it->second;
	}

	Socket* pSocket(*_ppSocket);
	if (!pSocket) // expired!
		return;


	if (_currentException) {
		pSocket->onError(_currentException);
		_currentException.set(Exception::NIL);
		if (*_ppSocket == NULL) // expired!
			return;
	}


	if (_currentError != 0) {
#if !defined(_WIN32)
		if (_currentError == NET_EINTR && pSocket->onConnection()) {
			char Temp;
			pSocket->receiveBytes(_currentException,&Temp,1); // to get the correct connection error!
		}
#endif
		if (!_currentException)
			Net::SetException(_currentException, _currentError);
		pSocket->onError(_currentException);
		_currentException.set(Exception::NIL);
		if (*_ppSocket == NULL) // expired!
			return;
	}

	if (_currentEvent == 0)
		return;


	/// now, connect, read, accept, or hangup event
#if defined(_WIN32)
	if (_currentEvent == FD_CONNECT) {
		pSocket->onConnection();
		if (_currentError==0 || *_ppSocket == NULL)
			return;
	}

#else
	do { // call onReadable at minimum one time (it can be an accept or hangup)
#endif
		pSocket->onReadable(_currentException,pSocket->available(_exSkip));
		if (_currentException) {
			if (*_ppSocket==NULL)
				return;
			pSocket->onError(_currentException);
			_currentException.set(Exception::NIL);
		}
#if !defined(_WIN32)
	} while (*_ppSocket && pSocket->available(_exSkip) > 0);  // just one time for windows, and fill available bytes for posix
	// while !exs because the socket can be deleted in pSocket->onReadable if on request the user call "close"
#endif
}

void SocketManager::requestHandle() {
	Exception ex;
	giveHandle(ex);
}

void SocketManager::run(Exception& exThread) {
	Exception& ex(_selfHandler ? exThread : _ex);
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
	if (_eventSystem == 0)
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
    #if defined(_OS_BSD)
        _eventSystem = kqueue();
    #else
		_eventSystem = epoll_create(1); // Argument is ignored, but has to be greater or equal to 1
    #endif
	if(_eventSystem<=0) {
		if(_eventFD>0)
			::close(_eventFD);
		if(readFD>0)
			::close(readFD);
		_eventSystem = 0;
		ex.set(Exception::NETWORK, name, " starting failed, impossible to manage sockets");
	} else {
        #if defined(_OS_BSD)
            struct kevent event;
            memset(&event, 0, sizeof(event));
            EV_SET(&event, readFD, EVFILT_READ, EV_ADD, 0, 0, NULL);
            kevent(_eventSystem, &event, 1, NULL, 0, NULL);
        #else
            // Add the event to terminate the epoll_wait!
            epoll_event event;
            memset(&event, 0, sizeof(event));
            event.events = EPOLLIN;
            event.data.fd = readFD;
            epoll_ctl(_eventSystem, EPOLL_CTL_ADD,readFD, &event);
        #endif
	}
#endif

	_initSignal.set();
	if (ex) { // here _eventSystem==0
		if (!Task::waitHandle())
			exThread = ex;
		return;
	}


#if defined(_WIN32)
	MSG msg;
	int result;
	while ((result = GetMessage(&msg, _eventSystem, 0, 0)) > 0) {

		if (msg.wParam == 0)
			continue;
		if (msg.message == 0) {
			// no mutex for ppSocket methods access because the socket has been removed already here!
			delete ((atomic<Socket*>*)msg.wParam);
			continue;
		} else if (msg.message != 104) // unknown message
			continue;
		_currentEvent = WSAGETSELECTEVENT(msg.lParam);
		_sockfd = msg.wParam;

		if (_currentEvent == FD_WRITE) {
			_currentEvent = 0;
			// protected for _sockets access
			lock_guard<recursive_mutex> lock(_mutex);
			auto& it = _sockets.find(_sockfd);
			if (it != _sockets.end()) {
				_ppSocket = it->second;
				(*_ppSocket)->flush(_currentException);
			}
		}
		// FD_CONNECT | FD_ACCEPT | FD_CLOSE | FD_READ | FD_WRITE
		if (_currentEvent==FD_READ && Socket::IOCTL(_exSkip, _sockfd, FIONREAD, 0) || _currentEvent>>1 || _currentException) {
			if (_currentEvent != FD_CLOSE) // in close case, it's not an error!
				_currentError = WSAGETSELECTERROR(msg.lParam);
			Task::waitHandle();
		}
		_currentException.set(Exception::NIL);
		_ppSocket = NULL;
		_sockfd = INVALID_SOCKET;
	}
	DestroyWindow(_eventSystem);
	if (result < 0)
		exThread = ex.set(Exception::NETWORK, name, " failed, impossible to manage sockets");
	
#elif defined(_OS_BSD)
    int count = _counter+1;
    vector<struct kevent> events(count);
    vector<Socket**> removedSockets;
    
    for(;;) {
    
        int results = kevent(_eventSystem, NULL, 0, &events[0], events.size(), NULL);
        
        if(results<0 && errno!=NET_EINTR) {
            Net::SetException(ex, Net::LastError());
			exThread = ex;
            break;
        }
        
        // for each ready socket
	int i=0;
        for(;i<results;++i) {
            struct kevent& event = events[i];
            
            if(event.ident==readFD) {
            
                if(event.flags & EV_EOF) {
                    i=-1; // termination signal!
                    break;
                }
                
                while (Socket::IOCTL(_exSkip,readFD, FIONREAD, 0)>=sizeof(_ppSocket) && read(readFD, &_ppSocket, sizeof(_ppSocket)) > 0) {
                    // no mutex for ppSocket methods access because the socket has been removed already here!
                    removedSockets.emplace_back(_ppSocket);
                }
                _ppSocket = NULL;
                continue;
            }
            
            _currentEvent = event.filter;
            if(event.flags&EV_ERROR) {
                _currentError = Net::LastError();
                _currentEvent &= ~EV_ERROR;
            }
            
            if(_currentEvent==EVFILT_WRITE) {
                lock_guard<recursive_mutex> lock(_mutex);
                _ppSocket = (Socket**)event.udata;
                Socket* pSocket(*_ppSocket);
                if(pSocket)
                    pSocket->flush(_currentException);
            }  else if(_currentException || _currentError>0 || _currentEvent>0) {
                if(!_ppSocket)
                    _ppSocket = (Socket**)event.udata;
                Task::waitHandle();
                _currentException.set(Exception::NIL);
                _currentError = 0;
            }
            
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
        for (Socket** ppSocket : removedSockets)
            delete ppSocket;
        removedSockets.clear();
    }
    ::close(readFD); // close reader pipe side
    ::close(_eventSystem); // close the system message
    
    for (Socket** ppSocket : removedSockets)
        delete ppSocket;
    removedSockets.clear();
#else
    int count = _counter+1;
	vector<epoll_event> events(count);
	vector<Socket**>	removedSockets;

	for(;;) {

		int results = epoll_wait(_eventSystem,&events[0],events.size(), -1);

		if(results<0 && errno!=NET_EINTR) {
			Net::SetException(ex, Net::LastError());
			exThread = ex;
			break;
		}

		// for each ready socket
		int i;
		for(i=0;i<results;++i) {
			epoll_event& event = events[i];

			if(event.data.fd==readFD) {

				if(event.events&EPOLLHUP) {
					i=-1; // termination signal!
					break;
				}

				while (Socket::IOCTL(_exSkip,readFD, FIONREAD, 0)>=sizeof(_ppSocket) && read(readFD, &_ppSocket, sizeof(_ppSocket)) > 0) {
					// no mutex for ppSocket methods access because the socket has been removed already here!
					removedSockets.emplace_back(_ppSocket);
				}
				_ppSocket = NULL;
				continue;	
			}
		
			_currentEvent = event.events;
			if(_currentEvent&EPOLLERR) {
                _currentError = Net::LastError();
				_currentEvent &= ~EPOLLERR;
			}

			if(_currentEvent&EPOLLOUT) {
				lock_guard<recursive_mutex> lock(_mutex);
				_ppSocket = (Socket**)event.data.ptr;
				Socket* pSocket(*_ppSocket);
				if(pSocket)
					pSocket->flush(_currentException);
				_currentEvent &= ~EPOLLOUT;
			}

			if(_currentException || _currentError>0 || _currentEvent>0) {
				if(!_ppSocket)
					_ppSocket = (Socket**)event.data.ptr;
				Task::waitHandle();
				_currentException.set(Exception::NIL);
				_currentError = 0;
			}

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
		for (Socket** ppSocket : removedSockets)
			delete ppSocket;
		removedSockets.clear();
	}
	::close(readFD);  // close reader pipe side
	::close(_eventSystem); // close the system message

	for (Socket** ppSocket : removedSockets)
		delete ppSocket;
	removedSockets.clear();
#endif


	_eventSystem = 0;
	_counter = 0;

	if (!Task::waitHandle()) { // to remove possible sockets remaing, or to set exception if(ex)
		lock_guard<recursive_mutex> lock(_mutex);
		if (!_sockets.empty()) {
			for (auto& it : _sockets)
				delete it.second;
			_sockets.clear();
			exThread.set(Exception::NETWORK, "TaskHandler of SocketManager is stopped, impossible to warn remaining sockets");
		}
	}

}

	


} // namespace Mona
