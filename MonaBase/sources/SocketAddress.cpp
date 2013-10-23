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


#include "Mona/SocketAddress.h"
#include "Mona/String.h"
#include "Mona/DNS.h"
#include <cstring>

using namespace std;


namespace Mona {


//
// SocketAddressCommon
//

class SocketAddressCommon : ObjectFix {
public:
	virtual const IPAddress&		host() const = 0;
	virtual UInt16					port() const = 0;
	virtual const struct sockaddr*  addr() const = 0;
	virtual IPAddress::Family		family() const = 0;
};


class IPv4SocketAddress : public SocketAddressCommon
{
public:
	IPv4SocketAddress() {
		memset(&_addr, 0, sizeof(_addr));
		_addr.sin_family = AF_INET;
		set_sin_len(&_addr);
		Exception ex; // will never throw!
		_host.set(ex, &_addr.sin_addr);
	}

	IPv4SocketAddress(const struct sockaddr_in* addr) {
		memcpy(&_addr, addr, sizeof(_addr));
		Exception ex; // will never throw!
		_host.set(ex, &_addr.sin_addr);
	}

	IPv4SocketAddress(const void* addr, UInt16 port) {
		memset(&_addr, 0, sizeof(_addr));
		_addr.sin_family = AF_INET;
		memcpy(&_addr.sin_addr, addr, sizeof(_addr.sin_addr));
		_addr.sin_port = port;
		Exception ex; // will never throw!
		_host.set(ex, &_addr.sin_addr);
	}

	IPAddress::Family family() const { return IPAddress::IPv4; }

	const IPAddress& host() const { return _host;}

	UInt16					port() const { return _addr.sin_port;}
	const struct sockaddr*	addr() const { return reinterpret_cast<const struct sockaddr*>(&_addr);}

private:
	struct sockaddr_in		_addr;
	mutable IPAddress		_host;
};


class IPv6SocketAddress: public SocketAddressCommon {
public:
	IPv6SocketAddress() {
		memset(&_addr, 0, sizeof(_addr));
		_addr.sin6_family = AF_INET6;
		set_sin6_len(&_addr);
		Exception ex; // will never throw!
		_host.set(ex,&_addr.sin6_addr);
	}
	IPv6SocketAddress(const struct sockaddr_in6* addr) {
		memcpy(&_addr, addr, sizeof(_addr));
		Exception ex; // will never throw!
		_host.set(ex, &_addr.sin6_addr, _addr.sin6_scope_id);
	}
	IPv6SocketAddress(const void* addr, UInt16 port, UInt32 scope = 0) {
		memset(&_addr, 0, sizeof(_addr));
		_addr.sin6_family = AF_INET6;
		set_sin6_len(&_addr);
		memcpy(&_addr.sin6_addr, addr, sizeof(_addr.sin6_addr));
		_addr.sin6_port = port;
		_addr.sin6_scope_id = scope;
		Exception ex; // will never throw!
		_host.set(ex, &_addr.sin6_addr, _addr.sin6_scope_id);
	}

	IPAddress::Family family() const { return IPAddress::IPv6; }

	const IPAddress& host() const { return _host;}

	UInt16					port() const { return _addr.sin6_port; }
	const struct sockaddr*	addr() const { return reinterpret_cast<const struct sockaddr*>(&_addr); }

private:
	struct sockaddr_in6		_addr;
	mutable IPAddress		_host;
};


//
// SocketAddress
//
SocketAddress SocketAddress::_Addressv4Wildcard;
SocketAddress SocketAddress::_Addressv6Wildcard(IPAddress::IPv6);

SocketAddress::SocketAddress(IPAddress::Family family) : _pAddress(family == IPAddress::IPv6 ? (SocketAddressCommon*)new IPv4SocketAddress() : (SocketAddressCommon*)new IPv6SocketAddress()) {
}


SocketAddress::~SocketAddress() {
	delete _pAddress;
}

void SocketAddress::set(const IPAddress& host, UInt16 port) {
	delete _pAddress;
	if (host.family() == IPAddress::IPv6)
		_pAddress = new IPv6SocketAddress(host.addr(), htons(port), host.scope());
	else
		_pAddress = new IPv4SocketAddress(host.addr(), htons(port));
	lock_guard<mutex>	lock(_mutex);
	_toString.clear();
}

bool SocketAddress::set(Exception& ex,const struct sockaddr* addr) {
	ASSERT_RETURN(addr!=NULL,false)
	SocketAddressCommon* pAddress(NULL);
	if (sizeof(addr) == sizeof(struct sockaddr_in))
		pAddress = new IPv4SocketAddress(reinterpret_cast<const struct sockaddr_in*>(addr));
	else if (sizeof(addr) == sizeof(struct sockaddr_in6))
		pAddress = new IPv6SocketAddress(reinterpret_cast<const struct sockaddr_in6*>(addr));
	else {
		ex.set(Exception::NETADDRESS, "Invalid socket address");
		return false;
	}
	delete _pAddress;
	_pAddress = pAddress;
	lock_guard<mutex>	lock(_mutex);
	_toString.clear();
	return true;
}

bool SocketAddress::set(Exception& ex,const string& hostAndPort) {
	ASSERT_RETURN(!hostAndPort.empty(),false);

	string host, port;
	auto it  = hostAndPort.begin();
	auto end = hostAndPort.end();
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
	return set(ex,host, resolveService(ex,port));
}

bool SocketAddress::set(Exception& ex,const string& host, UInt16 port) {
	IPAddress ip;
	Exception ignore;
	if (ip.set(ignore, host)) {
		set(ip, port);
		return true;
	}
	HostEntry entry;
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
	if (family() > address.family())
		return false;
	if (host() < address.host())
		return true;
	if (host() > address.host())
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

const struct sockaddr* SocketAddress::addr() const {
	return _pAddress->addr();
}

const string& SocketAddress::toString() const {
	lock_guard<mutex>	lock(_mutex);
	if (!_toString.empty())
		return _toString;

	if (_pAddress->family() == IPAddress::IPv6)
		_toString.append("[");

	_toString.append(host().toString());

	if (_pAddress->family() == IPAddress::IPv6)
		_toString.append("]");

	_toString.append(":");
	String::Append(_toString, port());
	return _toString;
}


UInt16 SocketAddress::resolveService(Exception& ex,const string& service) {
	UInt16 port = String::ToNumber<UInt16>(ex, service);
	if (!ex)
		return port;

#if defined(POCO_VXWORKS)
	ex.set(Exception::NETADDRESS, "Service ", service, " unknown");
#else
	if (!Net::InitializeNetwork(ex))
		return 0;
	struct servent* se = getservbyname(service.c_str(), NULL);
	if (se)
		return ntohs(se->s_port);
	ex.set(Exception::NETADDRESS, "Service ", service, " unknown");
#endif
	return 0;
}


}	// namespace Mona
