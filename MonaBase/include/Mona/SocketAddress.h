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
#include "Mona/IPAddress.h"


namespace Mona {


class SocketAddressCommon;

/// This class represents an internet (IP) endpoint/socket
/// address. The address can belong either to the
/// IPv4 or the IPv6 address family and consists of a
/// host address and a port number.
class SocketAddress : virtual Object {
public:
	/// Creates a wildcard (all zero) IPv4 SocketAddress
	SocketAddress(IPAddress::Family family = IPAddress::IPv4);
		
	SocketAddress(const IPAddress& host, UInt16 port);
	SocketAddress(const SocketAddress& other);

	void clear();

	void set(const SocketAddress& other);

	/// Creates a SocketAddress from an IP address and a port number.
	void set(const IPAddress& host, UInt16 port);

	/// set SocketAddress from an IP address and a port number.
	bool set(Exception& ex,const std::string& host, UInt16 port);

	/// set SocketAddress from an IP address and a service name or port number
	bool set(Exception& ex, const std::string& host, const std::string& port) { set(ex, host, resolveService(ex,port)); }

	/// set SocketAddress from an IP address or host name and a port number/service name
	bool set(Exception& ex, const std::string& hostAndPort);
	
	/// set SocketAddress from a native socket address
	bool set(Exception& ex, const struct sockaddr& addr);

	const IPAddress&		host() const;
	UInt16					port() const;
	IPAddress::Family		family() const;

	// Returns a string representation of the address
	const std::string&		toString() const;
	
	// Native socket address
	const struct sockaddr&	addr() const;
	
	bool operator < (const SocketAddress& address) const;
	bool operator == (const SocketAddress& address) const { return host() == address.host() && port() == address.port(); }
	bool operator != (const SocketAddress& address) const { return host() != address.host() || port() != address.port(); }

	// Returns a wildcard IPv4 or IPv6 address (0.0.0.0)
	static const SocketAddress& Wildcard(IPAddress::Family family = IPAddress::IPv4) { return family == IPAddress::IPv6 ? _Addressv6Wildcard : _Addressv4Wildcard; }

private:
	UInt16 resolveService(Exception& ex, const std::string& service);

	std::shared_ptr<SocketAddressCommon>	_pAddress;

	mutable std::mutex		_mutex;
	mutable std::string		_toString;

	static SocketAddress	_Addressv4Wildcard;
	static SocketAddress	_Addressv6Wildcard;
};



} // namespace Mona
