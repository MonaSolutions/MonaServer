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


#include "Mona/SocketAddress.h"
#include "Mona/String.h"
#include "Mona/DNS.h"
#include <cstring>

using namespace std;

namespace Mona {


//
// SocketAddressCommon
//

class SocketAddressCommon {
public:
	virtual const IPAddress&		host() const = 0;
	virtual UInt16					port() const = 0;
	virtual const struct sockaddr&  addr() const = 0;
	virtual IPAddress::Family		family() const = 0;
};


class IPv4SocketAddress : public SocketAddressCommon, virtual Object {
public:
	IPv4SocketAddress(const struct sockaddr_in* addr) : _host(addr->sin_addr) {
		memcpy(&_addr, addr, sizeof(_addr));
	}

	IPv4SocketAddress(const IPAddress& host, UInt16 port) : _host(host) {
		memset(&_addr, 0, sizeof(_addr));
		_addr.sin_family = AF_INET;
		NET_SOCKLEN size(0);
		memcpy(&_addr.sin_addr, host.addr(size), sizeof(_addr.sin_addr));
		_addr.sin_port = port;
	}

	IPAddress::Family family() const { return IPAddress::IPv4; }

	const IPAddress& host() const { return _host;}

	UInt16					port() const { return _addr.sin_port;}
	const struct sockaddr&	addr() const { return *reinterpret_cast<const struct sockaddr*>(&_addr);}

private:
	struct sockaddr_in	_addr;
	mutable IPAddress	_host;
};


class IPv6SocketAddress : public SocketAddressCommon, virtual Object {
public:
	IPv6SocketAddress(const struct sockaddr_in6* addr) : _host(addr->sin6_addr, addr->sin6_scope_id) {
		memcpy(&_addr, addr, sizeof(_addr));
	}
	IPv6SocketAddress(const IPAddress& host, UInt16 port, UInt32 scope = 0) : _host(host) {
		memset(&_addr, 0, sizeof(_addr));
		_addr.sin6_family = AF_INET6;
		set_sin6_len(&_addr);
		NET_SOCKLEN size(0);
		memcpy(&_addr.sin6_addr, host.addr(size), sizeof(_addr.sin6_addr));
		_addr.sin6_port = port;
		_addr.sin6_scope_id = scope;
	}

	IPAddress::Family family() const { return IPAddress::IPv6; }

	const IPAddress& host() const { return _host;}

	UInt16					port() const { return _addr.sin6_port; }
	const struct sockaddr&	addr() const { return *reinterpret_cast<const struct sockaddr*>(&_addr); }

private:
	struct sockaddr_in6	_addr;
	mutable IPAddress	_host;
};


//
// SocketAddress
//

class SocketWilcard : public SocketAddress {
public:
	SocketWilcard(IPAddress::Family family) {
		if (family == IPAddress::IPv6) {
			struct sockaddr_in6	addr;	
			memset(&addr, 0, sizeof(addr));
			addr.sin6_family = AF_INET6;
			set_sin6_len(&addr);
			set(*reinterpret_cast<const struct sockaddr*>(&addr));
		} else {
			struct sockaddr_in	addr;	
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			set_sin_len(&addr);
			set(*reinterpret_cast<const struct sockaddr*>(&addr));
		}
	}
};

static SocketWilcard _Addressv4Wildcard(IPAddress::IPv4);
static SocketWilcard _Addressv6Wildcard(IPAddress::IPv6);

SocketAddress::SocketAddress(IPAddress::Family family) : NullableObject(true),_pAddress(family == IPAddress::IPv6 ? _Addressv6Wildcard._pAddress : _Addressv4Wildcard._pAddress) {
}

SocketAddress::SocketAddress(const IPAddress& host, UInt16 port) {
	if (host.family() == IPAddress::IPv6)
		_pAddress.reset(new IPv6SocketAddress(host, htons(port), host.scope()));
	else
		_pAddress.reset(new IPv4SocketAddress(host, htons(port)));
}

SocketAddress::SocketAddress(const struct sockaddr& addr) {
	if (sizeof(addr) == sizeof(struct sockaddr_in6))
		_pAddress.reset(new IPv6SocketAddress(reinterpret_cast<const struct sockaddr_in6*>(&addr)));
	else
		_pAddress.reset(new IPv4SocketAddress(reinterpret_cast<const struct sockaddr_in*>(&addr)));
}

SocketAddress::SocketAddress(const SocketAddress& other) : _pAddress(other._pAddress) {
}

void SocketAddress::reset() {
	_pAddress = _pAddress->family() == IPAddress::IPv6 ? _Addressv6Wildcard._pAddress : _Addressv4Wildcard._pAddress;
	_isNull = true;
	_toString.clear();
}

void SocketAddress::set(const SocketAddress& other) {
	_pAddress = other._pAddress;
	_isNull = !other;
	_toString.clear();
}

void SocketAddress::set(const IPAddress& host, UInt16 port) {
	if (host.family() == IPAddress::IPv6)
		_pAddress.reset(new IPv6SocketAddress(host, htons(port), host.scope()));
	else
		_pAddress.reset(new IPv4SocketAddress(host, htons(port)));
	_isNull = host.isWildcard() && port==0;
	_toString.clear();
}

void SocketAddress::set(const struct sockaddr& addr) {
	if (sizeof(addr) == sizeof(struct sockaddr_in6))
		_pAddress.reset(new IPv6SocketAddress(reinterpret_cast<const struct sockaddr_in6*>(&addr)));
	else
		_pAddress.reset(new IPv4SocketAddress(reinterpret_cast<const struct sockaddr_in*>(&addr)));
	_isNull = _pAddress->host().isWildcard() && _pAddress->port()==0;
	_toString.clear();
}

bool SocketAddress::setIntern(Exception& ex,const string& hostAndPort,bool resolveHost) {
	ASSERT_RETURN(!hostAndPort.empty(),false);

	string host, port;
	auto it  = hostAndPort.begin();
	auto& end = hostAndPort.end();
	if (*it == '[') {
		++it;
		while (it != end && *it != ']')
			host += *it++;
		if (it == end) {
			ex.set(Exception::NETADDRESS,"Malformed IPv6 address ", hostAndPort);
			return false;
		}
		++it;
	} else {
		while (it != end && *it != ':')
			host += *it++;
	}
	if (it != end && *it == ':') {
		++it;
		while (it != end)
			port += *it++;
	}
	else {
		ex.set(Exception::NETADDRESS, "Missing port number in ", hostAndPort);
		return false;
	}
	return setIntern(ex,host, resolveService(ex,port),resolveHost);
}

bool SocketAddress::setIntern(Exception& ex,const string& host, UInt16 port,bool resolveHost) {
	IPAddress ip;
	Exception ignore;
	if (ip.set(ignore, host)) {
		set(ip, port);
		return true;
	}
	HostEntry entry;
	if (!resolveHost) {
		if (ignore)
			ex.set(ignore);
		return false;
	}
	if (!DNS::HostByName(ex, host, entry))
		return false;
	auto& addresses = entry.addresses();
	if (addresses.size() > 0) {
		// if we get both IPv4 and IPv6 addresses, prefer IPv4
		for (const IPAddress& address : addresses) {
			if (address.family() == IPAddress::IPv4) {
				set(address, port);
				return true;
			}
		}
		set(addresses.front(), port);
		return true;
	}
	ex.set(Exception::NETADDRESS, "No address found for the host ", host);
	return false;
}

bool SocketAddress::operator < (const SocketAddress& address) const {
	if (family() < address.family())
		return true;
	if (family() != address.family())
		return false;
	if (host() < address.host())
		return true;
	if (host() != address.host())
		return false;
	return (port() < address.port());
}

const IPAddress& SocketAddress::host() const {
	return _pAddress->host();
}

UInt16 SocketAddress::port() const {
	return ntohs(_pAddress->port());
}

IPAddress::Family SocketAddress::family() const {
	return _pAddress->family();
}

const struct sockaddr& SocketAddress::addr() const {
	return _pAddress->addr();
}

const string& SocketAddress::toString() const {
	if (!_toString.empty())
		return _toString;

	if (_pAddress->family() == IPAddress::IPv6)
		_toString.append("[");

	_toString.append(host().toString());

	if (_pAddress->family() == IPAddress::IPv6)
		_toString.append("]");

	_toString.append(":");
	return String::Append(_toString, port());
}


UInt16 SocketAddress::resolveService(Exception& ex,const string& service) {
	UInt16 port=0;
	if (String::ToNumber<UInt16>(service,port))
		return port;
	if (!Net::InitializeNetwork(ex))
		return 0;
	struct servent* se = getservbyname(service.c_str(), NULL);
	if (se)
		return ntohs(se->s_port);
	ex.set(Exception::NETADDRESS, "Service ", service, " unknown");
	return 0;
}


UInt16 SocketAddress::Split(const string& address,string& host) {
	auto found = address.find(':');
	host = address.substr(0, found);
	UInt16 port(0);
	if (found != string::npos) {
		address.substr(0, found);
		if (++found < address.size())
			String::ToNumber(address.substr(found), port);
	}
	return port;
}

const SocketAddress& SocketAddress::Wildcard(IPAddress::Family family) {
	return family == IPAddress::IPv6 ? _Addressv6Wildcard : _Addressv4Wildcard;
}

}	// namespace Mona
