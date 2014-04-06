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

#include "Mona/Mona.h"
#include "Mona/Exceptions.h"
#include <mutex>

#if defined(_WIN32)
#include <ws2tcpip.h>
#define NET_INVALID_SOCKET  INVALID_SOCKET
#define NET_SOCKET		    SOCKET
#define NET_SOCKLEN			int
#define NET_IOCTLREQUEST	int
#define NET_CLOSESOCKET(s)  closesocket(s)
#define NET_EINTR           WSAEINTR
#define NET_EACCES          WSAEACCES
#define NET_EFAULT          WSAEFAULT
#define NET_EINVAL          WSAEINVAL
#define NET_EMFILE          WSAEMFILE
#define NET_EAGAIN          WSAEWOULDBLOCK
#define NET_EWOULDBLOCK     WSAEWOULDBLOCK
#define NET_EINPROGRESS     WSAEINPROGRESS
#define NET_EALREADY        WSAEALREADY
#define NET_ENOTSOCK        WSAENOTSOCK
#define NET_EDESTADDRREQ    WSAEDESTADDRREQ
#define NET_EMSGSIZE        WSAEMSGSIZE
#define NET_EPROTOTYPE      WSAEPROTOTYPE
#define NET_ENOPROTOOPT     WSAENOPROTOOPT
#define NET_EPROTONOSUPPORT WSAEPROTONOSUPPORT
#define NET_ESOCKTNOSUPPORT WSAESOCKTNOSUPPORT
#define NET_ENOTSUP         WSAEOPNOTSUPP
#define NET_EPFNOSUPPORT    WSAEPFNOSUPPORT
#define NET_EAFNOSUPPORT    WSAEAFNOSUPPORT
#define NET_EADDRINUSE      WSAEADDRINUSE
#define NET_EADDRNOTAVAIL   WSAEADDRNOTAVAIL
#define NET_ENETDOWN        WSAENETDOWN
#define NET_ENETUNREACH     WSAENETUNREACH
#define NET_ENETRESET       WSAENETRESET
#define NET_ECONNABORTED    WSAECONNABORTED
#define NET_ECONNRESET      WSAECONNRESET
#define NET_ENOBUFS         WSAENOBUFS
#define NET_EISCONN         WSAEISCONN
#define NET_ENOTCONN        WSAENOTCONN
#define NET_ESHUTDOWN       WSAESHUTDOWN
#define NET_ETIMEDOUT       WSAETIMEDOUT
#define NET_ECONNREFUSED    WSAECONNREFUSED
#define NET_EHOSTDOWN       WSAEHOSTDOWN
#define NET_EHOSTUNREACH    WSAEHOSTUNREACH
#define NET_ESYSNOTREADY    WSASYSNOTREADY
#define NET_ENOTINIT        WSANOTINITIALISED
#define NET_HOST_NOT_FOUND  WSAHOST_NOT_FOUND
#define NET_TRY_AGAIN       WSATRY_AGAIN
#define NET_NO_RECOVERY     WSANO_RECOVERY
#define NET_NO_DATA         WSANO_DATA
#else
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#if _OS != _OS_HPUX
#include <sys/select.h>
#endif
#include <sys/ioctl.h>
#if defined(_OS_VMS)
#include <inet.h>
#else
#include <arpa/inet.h>
#endif
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#if defined(_OS_UNIX)
#include <net/if.h>
#endif
#if defined(sun) || defined(__APPLE__)
#include <sys/sockio.h>
#include <sys/filio.h>
#endif
#define NET_INVALID_SOCKET  -1
#define NET_SOCKET           int
#define NET_SOCKLEN          socklen_t
#if defined(_OS_BSD)
#define NET_IOCTLREQUEST     unsigned long
#else
#define NET_IOCTLREQUEST     int
#endif
#define NET_CLOSESOCKET(s)  ::close(s)
#define NET_EINTR           EINTR
#define NET_EACCES          EACCES
#define NET_EFAULT          EFAULT
#define NET_EINVAL          EINVAL
#define NET_EMFILE          EMFILE
#define NET_EAGAIN          EAGAIN
#define NET_EWOULDBLOCK     EWOULDBLOCK
#define NET_EINPROGRESS     EINPROGRESS
#define NET_EALREADY        EALREADY
#define NET_ENOTSOCK        ENOTSOCK
#define NET_EDESTADDRREQ    EDESTADDRREQ
#define NET_EMSGSIZE        EMSGSIZE
#define NET_EPROTOTYPE      EPROTOTYPE
#define NET_ENOPROTOOPT     ENOPROTOOPT
#define NET_EPROTONOSUPPORT EPROTONOSUPPORT
#if defined(ESOCKTNOSUPPORT)
#define NET_ESOCKTNOSUPPORT ESOCKTNOSUPPORT
#else
#define NET_ESOCKTNOSUPPORT -1
#endif
#define NET_ENOTSUP         ENOTSUP
#define NET_EPFNOSUPPORT    EPFNOSUPPORT
#define NET_EAFNOSUPPORT    EAFNOSUPPORT
#define NET_EADDRINUSE      EADDRINUSE
#define NET_EADDRNOTAVAIL   EADDRNOTAVAIL
#define NET_ENETDOWN        ENETDOWN
#define NET_ENETUNREACH     ENETUNREACH
#define NET_ENETRESET       ENETRESET
#define NET_ECONNABORTED    ECONNABORTED
#define NET_ECONNRESET      ECONNRESET
#define NET_ENOBUFS         ENOBUFS
#define NET_EISCONN         EISCONN
#define NET_ENOTCONN        ENOTCONN
#if defined(ESHUTDOWN)
#define NET_ESHUTDOWN       ESHUTDOWN
#else
#define NET_ESHUTDOWN       -2
#endif
#define NET_ETIMEDOUT       ETIMEDOUT
#define NET_ECONNREFUSED    ECONNREFUSED
#if defined(EHOSTDOWN)
#define NET_EHOSTDOWN       EHOSTDOWN
#else
#define NET_EHOSTDOWN       -3
#endif
#define NET_EHOSTUNREACH    EHOSTUNREACH
#define NET_ESYSNOTREADY    -4
#define NET_ENOTINIT        -5
#define NET_HOST_NOT_FOUND  HOST_NOT_FOUND
#define NET_TRY_AGAIN       TRY_AGAIN
#define NET_NO_RECOVERY     NO_RECOVERY
#define NET_NO_DATA         NO_DATA
#endif

#if defined(_OS_BSD) || ( _OS == _OS_TRU64) || (_OS ==  _OS_AIX) || ( _OS ==  _OS_IRIX) || (_OS == _OS_QNX) || (_OS == _OS_VXWORKS)
#define NET_HAVE_SALEN      1
#endif


#if !defined(AI_ADDRCONFIG)
#define AI_ADDRCONFIG 0
#endif



#if defined(NET_HAVE_SALEN)
#define	set_sa_len(pSA, len) (pSA)->sa_len   = (len)
#define set_sin_len(pSA)     (pSA)->sin_len  = sizeof(struct sockaddr_in)
#define set_sin6_len(pSA)    (pSA)->sin6_len = sizeof(struct sockaddr_in6)
#else
#define set_sa_len(pSA, len) (void) 0
#define set_sin_len(pSA)     (void) 0
#define set_sin6_len(pSA)    (void) 0
#endif


#ifndef INADDR_NONE
#define INADDR_NONE 0xFFFFFFFF
#endif


//
// Automatically link Base library.
//
#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#endif

namespace Mona {



class Net : virtual Static {
public:
#if defined(_WIN32)
	static int  LastError() { return WSAGetLastError(); }
#else
	static int  LastError() { return errno; }
#endif

	static std::string& GetErrorMessage(int error, std::string& message);

	template <typename ...Args>
	static void SetException(Exception& ex, int error, Args&&... args) {
		std::string message;
		ex.set(Exception::SOCKET, GetErrorMessage(error,message), args ...);
	}


#if defined(_WIN32)
	static bool InitializeNetwork(Exception& ex);
#else
	static bool InitializeNetwork(Exception& ex) {return true;}
#endif
};

} // namespace Mona
