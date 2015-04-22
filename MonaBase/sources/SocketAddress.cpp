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


using namespace std;

namespace Mona {


//
// SocketAddressCommon
//

class SocketAddressCommon : public virtual Object {
public:
	virtual const IPAddress&		host() const = 0;
	virtual UInt16					port() const = 0;
	virtual IPAddress::Family		family() const = 0;

	virtual const sockaddr* addr() const = 0;
	virtual NET_SOCKLEN		size() const = 0;
};


class IPv4SocketAddress : public SocketAddressCommon, public virtual Object {
public:
	IPv4SocketAddress(const struct sockaddr_in* addr) : _host(addr->sin_addr) {
		memcpy(&_addr, addr, sizeof(_addr));
	}

	IPv4SocketAddress(const IPAddress& host, UInt16 port) : _host(host) {
		memset(&_addr, 0, sizeof(_addr));
		_addr.sin_family = AF_INET;
		memcpy(&_addr.sin_addr, host.addr(), sizeof(_addr.sin_addr));
		_addr.sin_port = port;
	}

	IPAddress::Family family() const { return IPAddress::IPv4; }

	const IPAddress& host() const { return _host;}
	UInt16			 port() const { return _addr.sin_port;}

	const sockaddr*	 addr() const { return (sockaddr*)&_addr;}
	NET_SOCKLEN		 size() const { return sizeof(_addr); }

private:
	struct sockaddr_in	_addr;
	mutable IPAddress	_host;
};


class IPv6SocketAddress : public SocketAddressCommon, public virtual Object {
public:
	IPv6SocketAddress(const struct sockaddr_in6* addr) : _host(addr->sin6_addr, addr->sin6_scope_id) {
		memcpy(&_addr, addr, sizeof(_addr));
	}
	IPv6SocketAddress(const IPAddress& host, UInt16 port, UInt32 scope = 0) : _host(host) {
		memset(&_addr, 0, sizeof(_addr));
		_addr.sin6_family = AF_INET6;
		set_sin6_len(&_addr);
		memcpy(&_addr.sin6_addr, host.addr(), sizeof(_addr.sin6_addr));
		_addr.sin6_port = port;
		_addr.sin6_scope_id = scope;
	}

	IPAddress::Family family() const { return IPAddress::IPv6; }

	const IPAddress& host() const { return _host;}

	UInt16			 port() const { return _addr.sin6_port; }

	const sockaddr*	 addr() const { return (sockaddr*)&_addr;}
	NET_SOCKLEN		 size() const { return sizeof(_addr); }

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

SocketAddress::SocketAddress(IPAddress::Family family) : _pAddress(family == IPAddress::IPv6 ? _Addressv6Wildcard._pAddress : _Addressv4Wildcard._pAddress) {
}

SocketAddress::SocketAddress(const IPAddress& host, UInt16 port) {
	if (host.family() == IPAddress::IPv6)
		_pAddress.reset(new IPv6SocketAddress(host, htons(port), host.scope()));
	else
		_pAddress.reset(new IPv4SocketAddress(host, htons(port)));
}

SocketAddress::SocketAddress(const struct sockaddr& addr) {
	if (addr.sa_family == AF_INET6)
		_pAddress.reset(new IPv6SocketAddress(reinterpret_cast<const struct sockaddr_in6*>(&addr)));
	else
		_pAddress.reset(new IPv4SocketAddress(reinterpret_cast<const struct sockaddr_in*>(&addr)));
}

SocketAddress::SocketAddress(const SocketAddress& other) : _pAddress(other._pAddress),_toString(other._toString) {
}

void SocketAddress::clear() {
	_pAddress = _pAddress->family() == IPAddress::IPv6 ? _Addressv6Wildcard._pAddress : _Addressv4Wildcard._pAddress;
	_toString.clear();
}

SocketAddress& SocketAddress::set(const SocketAddress& other) {
	_pAddress = other._pAddress;
	_toString = other._toString;
	return *this;
}

SocketAddress& SocketAddress::set(const IPAddress& host, UInt16 port) {
	if (host.family() == IPAddress::IPv6)
		_pAddress.reset(new IPv6SocketAddress(host, htons(port), host.scope()));
	else
		_pAddress.reset(new IPv4SocketAddress(host, htons(port)));
	_toString.clear();
	return *this;
}

SocketAddress& SocketAddress::set(const struct sockaddr& addr) {
	if (addr.sa_family == AF_INET6)
		_pAddress.reset(new IPv6SocketAddress(reinterpret_cast<const struct sockaddr_in6*>(&addr)));
	else
		_pAddress.reset(new IPv4SocketAddress(reinterpret_cast<const struct sockaddr_in*>(&addr)));
	_toString.clear();
	return *this;
}

bool SocketAddress::setIntern(Exception& ex,const char* hostAndPort,bool resolveHost) {
	ASSERT_RETURN(hostAndPort,false);

	const char* colon(strrchr(hostAndPort,':'));
	if (!colon) {
		ex.set(Exception::NETPORT, "Missing port number in ", hostAndPort);
		return false;
	}
	(char&)*colon = 0;
	bool result(setIntern(ex,hostAndPort, resolveService(ex,colon+1),resolveHost));
	(char&)*colon = ':';
	return result;
}

bool SocketAddress::setIntern(Exception& ex,const char* host, UInt16 port,bool resolveHost) {
	IPAddress ip;
	if (resolveHost) {
		if (!ip.setWithDNS(ex, host))
			return false;
	} else if(!ip.set(ex, host))
		return false;
	set(ip, port);
	return true;
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

const sockaddr* SocketAddress::addr() const {
	return _pAddress->addr();
}

NET_SOCKLEN SocketAddress::size() const {
	return _pAddress->size();
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


UInt16 SocketAddress::resolveService(Exception& ex,const char* service) {
	UInt16 port=0;
	if (String::ToNumber<UInt16>(service,port))
		return port;
	if (!Net::InitializeNetwork(ex))
		return 0;
	struct servent* se = getservbyname(service, NULL);
	if (se)
		return ntohs(se->s_port);
	ex.set(Exception::NETPORT, "Service ", service, " unknown");
	return 0;
}


const SocketAddress& SocketAddress::Wildcard(IPAddress::Family family) {
	return family == IPAddress::IPv6 ? _Addressv6Wildcard : _Addressv4Wildcard;
}

UInt16 SocketAddress::SplitLiteral(const char* value,string& host) {
	UInt16 port(0);
	host.assign(value);
	const char* colon(strchr(value,':'));
	if (colon && String::ToNumber(colon+1, port)) // check if it's really a marker port colon (could be a IPv6 colon)
		host.resize(colon-value);
	return port;
}

}	// namespace Mona
