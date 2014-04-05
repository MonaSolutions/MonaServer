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

#include "Mona/Net.h"

using namespace std;

namespace Mona {


string& Net::GetErrorMessage(int error, string& message) {
	switch (error) {
	case NET_ESYSNOTREADY:
		message.assign("Net subsystem not ready");
		break;
	case NET_ENOTINIT:
		message.assign("Net subsystem not initialized");
		break;
	case NET_EINTR:
		message.assign("Interrupted");
		break;
	case NET_EACCES:
		message.assign("Permission denied");
		break;
	case NET_EFAULT:
		message.assign("Bad address");
		break;
	case NET_EINVAL:
		message.assign("Invalid argument");
		break;
	case NET_EMFILE:
		message.assign("Too many open files");
		break;
	case NET_EWOULDBLOCK:
		message.assign("Operation would block");
		break;
	case NET_EINPROGRESS:
		message.assign("Operation now in progress");
		break;
	case NET_EALREADY:
		message.assign("Operation already in progress");
		break;
	case NET_ENOTSOCK:
		message.assign("Socket operation attempted on non-socket");
		break;
	case NET_EDESTADDRREQ:
		message.assign("Destination address required");
		break;
	case NET_EMSGSIZE:
		message.assign("Message too long");
		break;
	case NET_EPROTOTYPE:
		message.assign("Wrong protocol type");
		break;
	case NET_ENOPROTOOPT:
		message.assign("Protocol not available");
		break;
	case NET_EPROTONOSUPPORT:
		message.assign("Protocol not supported");
		break;
	case NET_ESOCKTNOSUPPORT:
		message.assign("Socket type not supported");
		break;
	case NET_ENOTSUP:
		message.assign("Operation not supported");
		break;
	case NET_EPFNOSUPPORT:
		message.assign("Protocol family not supported");
		break;
	case NET_EAFNOSUPPORT:
		message.assign("Address family not supported");
		break;
	case NET_EADDRINUSE:
		message.assign("Address already in use");
		break;
	case NET_EADDRNOTAVAIL:
		message.assign("Cannot assign requested address");
		break;
	case NET_ENETDOWN:
		message.assign("Network is down");
		break;
	case NET_ENETUNREACH:
		message.assign("Network is unreachable");
		break;
	case NET_ENETRESET:
		message.assign("Network dropped connection on reset");
		break;
	case NET_ECONNABORTED:
		message.assign("Connection aborted");
		break;
	case NET_ECONNRESET:
		message.assign("Connection reseted");
		break;
	case NET_ENOBUFS:
		message.assign("No buffer space available");
		break;
	case NET_EISCONN:
		message.assign("Socket is already connected");
		break;
	case NET_ENOTCONN:
		message.assign("Socket is not connected");
		break;
	case NET_ESHUTDOWN:
		message.assign("Cannot send after socket shutdown");
		break;
	case NET_ETIMEDOUT:
		message.assign("Timeout");
		break;
	case NET_ECONNREFUSED:
		message.assign("Connection refused");
		break;
	case NET_EHOSTDOWN:
		message.assign("Host is down");
		break;
	case NET_EHOSTUNREACH:
		message.assign("No route to host");
		break;
#if !defined(_WIN32)
	case EPIPE:
		message.assign("Broken pipe");
		break;
#endif
	default:
		message.assign("I/O error");
	}
	return message;
}

#if defined(_WIN32)
static volatile bool	Initialized(false);
static mutex			Mutex;

bool Net::InitializeNetwork(Exception& ex) {
	lock_guard<mutex> lock(Mutex);
	if (Initialized)
		return true;
	WORD    version = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(version, &data) != 0) {
		ex.set(Exception::NETWORK, "Failed to initialize network subsystem");
		return false;
	}
	return Initialized = true;
}

class NetUninitializer {
public:
	~NetUninitializer() {
		if (!Initialized)
			return;
		WSACleanup();
	}
};


NetUninitializer uninitializer;
#endif

} // namespace Mona
