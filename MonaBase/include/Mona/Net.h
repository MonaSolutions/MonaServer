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

#include "Mona/Mona.h"
#include "Mona/Exceptions.h"
#include <mutex>

#if defined(_WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>
#define SOCKLEN       		 int
#define IOCTL_REQUEST 		 int
#define CLOSESOCKET(s)  	 closesocket(s)
#undef EINTR
#define EINTR           	 WSAEINTR
#undef EACCES
#define EACCES          	 WSAEACCES
#undef EFAULT
#define EFAULT          	 WSAEFAULT
#undef EINVAL
#define EINVAL         		 WSAEINVAL
#undef EMFILE
#define EMFILE          	 WSAEMFILE
#undef EAGAIN
#define EAGAIN               WSAEWOULDBLOCK
#undef EWOULDBLOCK
#define EWOULDBLOCK     	 WSAEWOULDBLOCK
#undef EINPROGRESS
#define EINPROGRESS     	 WSAEINPROGRESS
#undef EALREADY
#define EALREADY        	 WSAEALREADY
#undef ENOTSOCK
#define ENOTSOCK        	 WSAENOTSOCK
#undef EDESTADDRREQ
#define EDESTADDRREQ    	 WSAEDESTADDRREQ
#undef EMSGSIZE
#define EMSGSIZE        	 WSAEMSGSIZE
#undef EPROTOTYPE
#define EPROTOTYPE      	 WSAEPROTOTYPE
#undef ENOPROTOOPT
#define ENOPROTOOPT     	 WSAENOPROTOOPT
#undef EPROTONOSUPPORT
#define EPROTONOSUPPORT 	 WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT 	 WSAESOCKTNOSUPPORT
#undef ENOTSUP
#define ENOTSUP         	 WSAEOPNOTSUPP
#define EPFNOSUPPORT    	 WSAEPFNOSUPPORT
#undef EAFNOSUPPORT
#define EAFNOSUPPORT    	 WSAEAFNOSUPPORT
#undef EADDRINUSE
#define EADDRINUSE      	 WSAEADDRINUSE
#undef EADDRNOTAVAIL
#define EADDRNOTAVAIL   	 WSAEADDRNOTAVAIL
#undef ENETDOWN
#define ENETDOWN        	 WSAENETDOWN
#undef ENETUNREACH
#define ENETUNREACH     	 WSAENETUNREACH
#undef ENETRESET
#define ENETRESET       	 WSAENETRESET
#undef ECONNABORTED
#define ECONNABORTED    	 WSAECONNABORTED
#undef ECONNRESET
#define ECONNRESET     	     WSAECONNRESET
#undef ENOBUFS
#define ENOBUFS         	 WSAENOBUFS
#undef EISCONN
#define EISCONN         	 WSAEISCONN
#undef ENOTCONN
#define ENOTCONN        	 WSAENOTCONN
#define ESHUTDOWN       	 WSAESHUTDOWN
#undef ETIMEDOUT
#define ETIMEDOUT       	 WSAETIMEDOUT
#undef ECONNREFUSED
#define ECONNREFUSED    	 WSAECONNREFUSED
#define EHOSTDOWN       	 WSAEHOSTDOWN
#undef EHOSTUNREACH
#define EHOSTUNREACH    	 WSAEHOSTUNREACH
#define ESYSNOTREADY    	 WSASYSNOTREADY
#define ENOTINIT        	 WSANOTINITIALISED
#define HOST_NOT_FOUND  	 WSAHOST_NOT_FOUND
#define TRY_AGAIN       	 WSATRY_AGAIN
#define NO_RECOVERY     	 WSANO_RECOVERY
#define NO_DATA        	 	 WSANO_DATA

#elif defined(POCO_VXWORKS)

#include <hostLib.h>
#include <ifLib.h>
#include <inetLib.h>
#include <ioLib.h>
#include <resolvLib.h>
#include <types.h>
#include <socket.h>
#include <netinet/tcp.h>
#define INVALID_SOCKET  	 -1
#define SOCKET		         int
#define SOCKLEN	         	 int
#define IOCTL_REQUEST 		 int
#define CLOSESOCKET(s)  	 ::close(s)

#elif defined(POCO_OS_FAMILY_UNIX) || defined(POCO_OS_FAMILY_VMS)

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#if POCO_OS != POCO_OS_HPUX
#include <sys/select.h>
#endif
#include <sys/ioctl.h>
#if defined(POCO_OS_FAMILY_VMS)
#include <inet.h>
#else
#include <arpa/inet.h>
#endif
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#if defined(POCO_OS_FAMILY_UNIX)
#include <net/if.h>
#endif
#if defined(sun) || defined(__APPLE__)
#include <sys/sockio.h>
#include <sys/filio.h>
#endif

#define INVALID_SOCKET  	 -1
#define SOCKET		         int
#define SOCKLEN       		 socklen_t
#if defined(POCO_OS_FAMILY_BSD)
#define IOCTL_REQUEST 		 unsigned long
#else
#define IOCTL_REQUEST 		 int
#endif
#define CLOSESOCKET(s)  	 ::close(s)
#if !defined(ESOCKTNOSUPPORT)
#define POCO_ESOCKTNOSUPPORT -1
#endif
#if !defined(ESHUTDOWN)
#define POCO_ESHUTDOWN       -2
#endif
#if !defined(EHOSTDOWN)
#define POCO_EHOSTDOWN       -3
#endif
#define ESYSNOTREADY    	 -4
#define ENOTINIT        	 -5

#endif



#if defined(POCO_OS_FAMILY_BSD) || (POCO_OS == POCO_OS_TRU64) || (POCO_OS == POCO_OS_AIX) || (POCO_OS == POCO_OS_IRIX) || (POCO_OS == POCO_OS_QNX) || (POCO_OS == POCO_OS_VXWORKS)
#define POCO_HAVE_SALEN      1
#endif


#if POCO_OS != POCO_OS_VXWORKS && !defined(POCO_NET_NO_ADDRINFO)
#define POCO_HAVE_ADDRINFO   1
#endif


#if defined(POCO_HAVE_ADDRINFO)
#if !defined(AI_ADDRCONFIG)
#define AI_ADDRCONFIG 0
#endif
#endif


#if defined(POCO_HAVE_SALEN)
#define set_sa_len(pSA, len) (pSA)->sa_len   = (len)
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


namespace Mona {



class Net : Fix {
public:
#if defined(_WIN32)
	static bool InitializeNetwork(Exception& ex);
#else
	static bool InitializeNetwork(Exception& ex) {return true;}
#endif
};

} // namespace Mona
