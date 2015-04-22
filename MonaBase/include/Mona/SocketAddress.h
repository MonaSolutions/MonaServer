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
#include "Mona/IPAddress.h"


namespace Mona {


class SocketAddressCommon;

/// This class represents an internet (IP) endpoint/socket
/// address. The address can belong either to the
/// IPv4 or the IPv6 address family and consists of a
/// host address and a port number.
class SocketAddress : public virtual NullableObject {
public:
	/// Creates a wildcard (all zero) IPv4 SocketAddress
	SocketAddress(IPAddress::Family family = IPAddress::IPv4);
		
	/// Create/Set SocketAddress from a native socket address
	SocketAddress(const struct sockaddr& addr);
	SocketAddress& set(const struct sockaddr& addr);
	
	SocketAddress(const SocketAddress& other);
	SocketAddress& set(const SocketAddress& other);
	SocketAddress& operator=(const SocketAddress& other) { return set(other); }

	/// Creates a SocketAddress from an IP address and a port number.
	SocketAddress(const IPAddress& host, UInt16 port);
	SocketAddress& set(const IPAddress& host, UInt16 port);
	SocketAddress& set(Exception& ex, const IPAddress& host, const std::string& port) { return set(host, resolveService(ex, port.c_str())); }
	SocketAddress& set(Exception& ex, const IPAddress& host, const char* port) { return set(host, resolveService(ex, port)); }
	
	/// set SocketAddress from an IP address and a port number.
	bool set(Exception& ex, const std::string& host, UInt16 port) { return setIntern(ex, host.c_str(), port,false); }
	bool set(Exception& ex, const char* host, UInt16 port) { return setIntern(ex, host, port,false); }
	bool setWithDNS(Exception& ex, const std::string& host, UInt16 port) { return setIntern(ex, host.c_str(), port,true); }
	bool setWithDNS(Exception& ex, const char* host, UInt16 port) { return setIntern(ex, host, port,true); }

	/// set SocketAddress from an IP address and a service name or port number
	bool set(Exception& ex, const std::string& host, const std::string& port) { return setIntern(ex, host.c_str(), resolveService(ex,port.c_str()),false); }
	bool set(Exception& ex, const char* host,		 const std::string& port) { return setIntern(ex, host, resolveService(ex,port.c_str()),false); }
	bool set(Exception& ex, const std::string& host, const char* port) { return setIntern(ex, host.c_str(), resolveService(ex,port),false); }
	bool set(Exception& ex, const char* host,		 const char* port) { return setIntern(ex, host, resolveService(ex,port),false); }
	bool setWithDNS(Exception& ex, const std::string& host, const std::string& port) { return setIntern(ex, host.c_str(), resolveService(ex,port.c_str()),true); }
	bool setWithDNS(Exception& ex, const char* host,		const std::string& port) { return setIntern(ex, host, resolveService(ex,port.c_str()),true); }
	bool setWithDNS(Exception& ex, const std::string& host, const char* port) { return setIntern(ex, host.c_str(), resolveService(ex,port),true); }
	bool setWithDNS(Exception& ex, const char* host,		const char* port) { return setIntern(ex, host, resolveService(ex,port),true); }

	/// set SocketAddress from an IP address or host name and a port number/service name
	bool set(Exception& ex, const std::string& hostAndPort) { return setIntern(ex, hostAndPort.c_str(), false); }
	bool set(Exception& ex, const char* hostAndPort) { return setIntern(ex, hostAndPort, false); }
	bool setWithDNS(Exception& ex, const std::string& hostAndPort) { return setIntern(ex, hostAndPort.c_str(), true); }
	bool setWithDNS(Exception& ex, const char* hostAndPort) { return setIntern(ex, hostAndPort, true); }
	
	void clear();

	const IPAddress&		host() const;
	UInt16					port() const;
	IPAddress::Family		family() const;

	// Returns a string representation of the address
	const std::string&		toString() const;
	
	// Native socket address
	const sockaddr*	addr() const;
	NET_SOCKLEN		size() const;
	
	bool operator < (const SocketAddress& address) const;
	bool operator == (const SocketAddress& address) const { return port() == address.port() && host() == address.host(); }
	bool operator != (const SocketAddress& address) const { return port() != address.port() || host() != address.host(); }
	
	operator bool() const { return host() || port()>0; }

	// Returns a wildcard IPv4 or IPv6 address (0.0.0.0)
	static const SocketAddress& Wildcard(IPAddress::Family family = IPAddress::IPv4);

	static UInt16 SplitLiteral(const char* value, std::string& host);
	static UInt16 SplitLiteral(const std::string& value, std::string& host) { return SplitLiteral(value.data(),host); }

private:
	bool setIntern(Exception& ex,const char* host, UInt16 port,bool resolveHost);
	bool setIntern(Exception& ex,const char* hostAndPort, bool resolveHost);

	UInt16 resolveService(Exception& ex, const char* service);

	std::shared_ptr<SocketAddressCommon>	_pAddress;
	mutable std::string						_toString;
};



} // namespace Mona
