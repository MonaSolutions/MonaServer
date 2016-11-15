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



#include "Mona/IPAddress.h"
#include "Mona/String.h"
#include "Mona/DNS.h"
#include "Mona/Util.h"
#if defined(WIN32)
	#include <Iphlpapi.h>
#else
	#include <ifaddrs.h>
#endif

using namespace std;


namespace Mona {

static const char* LocalhostV4("127.0.0.1"); // to accelerate the parse
static const char* LocalhostV6("::1"); // to accelerate the parse

/// Returns the length of the mask (number of bits set in val).
/// The val should be either all zeros or two contiguos areas of 1s and 0s. 
/// The algorithm ignores invalid non-contiguous series of 1s and treats val 
/// as if all bits between MSb and last non-zero bit are set to 1.
UInt8 MaskBits(UInt32 val, UInt8 size) {
	UInt8 count = 0;
	if (val) {
		val = (val ^ (val - 1)) >> 1;
		for (count = 0; val; ++count)
			val >>= 1;
	} else
		count = size;
	return size - count;
}

class IPAddressCommon : public virtual Object {
public:
	virtual const void* addr() const = 0;
	virtual NET_SOCKLEN size() const = 0;

	virtual const string& toString() const = 0;
	virtual IPAddress::Family family() const = 0;
	virtual UInt32 scope() const = 0;
	virtual bool isWildcard() const	= 0;
	virtual bool isBroadcast() const = 0;
	virtual bool isAnyBroadcast() const = 0;
	virtual bool isLoopback() const = 0;
	virtual bool isMulticast() const = 0;
	virtual bool isLinkLocal() const = 0;
	virtual bool isSiteLocal() const = 0;
	virtual bool isIPv4Mapped() const = 0;
	virtual bool isIPv4Compatible() const = 0;
	virtual bool isWellKnownMC() const = 0;
	virtual bool isNodeLocalMC() const = 0;
	virtual bool isLinkLocalMC() const = 0;
	virtual bool isSiteLocalMC() const = 0;
	virtual bool isOrgLocalMC() const = 0;
	virtual bool isGlobalMC() const = 0;
	virtual UInt8 prefixLength() const = 0;
};


class IPv4Address : public IPAddressCommon {
public:

	IPv4Address(const in_addr& addr) {
		memcpy(&_addr, &addr, sizeof(_addr));
	}

	IPv4Address(const in_addr& addr, const in_addr& mask, const in_addr& set) {
		memcpy(&_addr,&addr,sizeof(addr)) ;
		_addr.s_addr &= mask.s_addr;
		_addr.s_addr |= set.s_addr & ~mask.s_addr;
	}

	const string& toString() const {
		lock_guard<mutex> lock(_mutex);
		if (!_toString.empty())
			return _toString;
		const UInt8* bytes = reinterpret_cast<const UInt8*>(&_addr);
		return String::Format(_toString, bytes[0], '.', bytes[1], '.', bytes[2], '.', bytes[3]);
	}

	const void*		addr() const { return &_addr; }
	NET_SOCKLEN		size() const { return sizeof(_addr); }

	IPAddress::Family	family() const {return IPAddress::IPv4;}

	UInt32	scope() const { return 0; }
	bool	isWildcard() const { return _addr.s_addr == INADDR_ANY; }
	bool	isBroadcast() const { return _addr.s_addr == INADDR_NONE; }
	bool	isAnyBroadcast() const { return _addr.s_addr == INADDR_NONE || (ntohl(_addr.s_addr) & 0x000000FF) == 0x000000FF; }
	bool	isLoopback() const { return (ntohl(_addr.s_addr) & 0xFF000000) == 0x7F000000; } // 127.0.0.1 to 127.255.255.255 
	bool	isMulticast() const { return (ntohl(_addr.s_addr) & 0xF0000000) == 0xE0000000; } // 224.0.0.0/24 to 239.0.0.0/24 
	bool	isLinkLocal() const { return (ntohl(_addr.s_addr) & 0xFFFF0000) == 0xA9FE0000; } // 169.254.0.0/16

	bool	isSiteLocal() const {
		UInt32 addr = ntohl(_addr.s_addr);
		return (addr & 0xFF000000) == 0x0A000000 ||        // 10.0.0.0/24
		       (addr & 0xFFFF0000) == 0xC0A80000 ||        // 192.68.0.0/16
		       (addr >= 0xAC100000 && addr <= 0xAC1FFFFF); // 172.16.0.0 to 172.31.255.255
	}
	
	bool isIPv4Compatible() const	{ return true; }
	bool isIPv4Mapped() const { return true; }
	bool isWellKnownMC() const { return (ntohl(_addr.s_addr) & 0xFFFFFF00) == 0xE0000000; } // 224.0.0.0/8
	bool isNodeLocalMC() const { return false; }
	bool isLinkLocalMC() const { return (ntohl(_addr.s_addr) & 0xFF000000) == 0xE0000000; } // 244.0.0.0/24
	bool isSiteLocalMC() const { return (ntohl(_addr.s_addr) & 0xFFFF0000) == 0xEFFF0000; } // 239.255.0.0/16
	bool isOrgLocalMC() const { return (ntohl(_addr.s_addr) & 0xFFFF0000) == 0xEFC00000; } // 239.192.0.0/16

	bool isGlobalMC() const {
		UInt32 addr = ntohl(_addr.s_addr);
		return addr >= 0xE0000100 && addr <= 0xEE000000; // 224.0.1.0 to 238.255.255.255
	}

	UInt8 prefixLength() const {
		return MaskBits(ntohl(_addr.s_addr), 32);
	}

	static IPv4Address* Parse(Exception& ex,const char* address) {
		if (!address || !Net::InitializeNetwork(ex))
			return 0;
		if (String::ICompare(address, "localhost") == 0)
			address = LocalhostV4;
		struct in_addr ia;
#if defined(_WIN32) 
		ia.s_addr = inet_addr(address);
		if (ia.s_addr == INADDR_NONE && strcmp(address,"255.255.255.255")!=0)
			return 0;
		return new IPv4Address(ia);
#else
		if (inet_aton(address, &ia))
            return new IPv4Address(ia);
		return 0;
#endif
	}
	

private:
	struct	in_addr		_addr;	
	mutable string		_toString;
	mutable std::mutex	_mutex;
};


class IPv6Address : public IPAddressCommon {
public:
	IPv6Address(const in6_addr& addr, UInt32 scope = 0) : _scope(scope) {
		memcpy(&_addr, &addr, sizeof(_addr));
	}

	const string& toString() const {
		lock_guard<mutex> lock(_mutex);
		if (!_toString.empty())
			return _toString;
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		if (isIPv4Compatible() || isIPv4Mapped()) {
			const UInt8* bytes = reinterpret_cast<const UInt8*>(&_addr);
			return String::Format(_toString, words[5] == 0 ? "::" : "::FFFF:", bytes[12], '.', bytes[13], '.', bytes[14], '.', bytes[15]);
		}
		bool zeroSequence = false;
		int i = 0;
		while (i < 8) {
			if (!zeroSequence && words[i] == 0) {
				int zi = i;
				while (zi < 8 && words[zi] == 0)
					++zi;
				if (zi > i + 1) {
					i = zi;
					_toString.append(":");
					zeroSequence = true;
				}
			}
			if (i > 0)
				_toString.append(":");
			if (i < 8)
				Util::FormatHex((const UInt8*)&words[i++],2,_toString,Util::HEX_TRIM_LEFT | Util::HEX_APPEND);
		}
		if (_scope > 0) {
			_toString.append("%");
#if defined(_WIN32)
			String::Append(_toString, _scope);
#else
            char buffer[IFNAMSIZ];
            if (if_indextoname(_scope, buffer))
                _toString.append(buffer);
            else
                String::Append(_toString, _scope);
#endif
		}
		return _toString;
	}
	
	

	IPAddress::Family	family() const {return IPAddress::IPv6;}

	const void*			addr() const { return &_addr; }
	NET_SOCKLEN			size() const { return sizeof(_addr); }

	UInt32				scope() const {return _scope;}
	bool				isBroadcast() const {return false;}
	bool				isAnyBroadcast() const { return false; }

	bool isWildcard() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0 &&
			words[4] == 0 && words[5] == 0 && words[6] == 0 && words[7] == 0;
	}
	bool isLoopback() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0 && 
		       words[4] == 0 && words[5] == 0 && words[6] == 0 && ntohs(words[7]) == 0x0001;
	}
	bool isMulticast() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (ntohs(words[0]) & 0xFFE0) == 0xFF00;
	}
	bool isLinkLocal() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (ntohs(words[0]) & 0xFFE0) == 0xFE80;
	}
	bool isSiteLocal() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (ntohs(words[0]) & 0xFFE0) == 0xFEC0;
	}
	bool isIPv4Compatible() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0 && words[4] == 0 && words[5] == 0;
	}
	bool isIPv4Mapped() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return words[0] == 0 && words[1] == 0 && words[2] == 0 && words[3] == 0 && words[4] == 0 && ntohs(words[5]) == 0xFFFF;
	}
	bool isWellKnownMC() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (ntohs(words[0]) & 0xFFF0) == 0xFF00;
	}
	bool isNodeLocalMC() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (ntohs(words[0]) & 0xFFEF) == 0xFF01;
	}
	bool isLinkLocalMC() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (ntohs(words[0]) & 0xFFEF) == 0xFF02;
	}
	bool isSiteLocalMC() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (ntohs(words[0]) & 0xFFEF) == 0xFF05;
	}
	bool isOrgLocalMC() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (ntohs(words[0]) & 0xFFEF) == 0xFF08;
	}
	bool isGlobalMC() const {
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		return (ntohs(words[0]) & 0xFFEF) == 0xFF0F;
	}

	UInt8 prefixLength() const {
		UInt8 bits = 0;
		UInt8 bitPos = 128;
		const UInt16* words = reinterpret_cast<const UInt16*>(&_addr);
		for (int i = 7; i >= 0; --i) {
			if ((bits = MaskBits(ntohs(words[i]), 16)))
				return (bitPos - (16 - bits));
			bitPos -= 16;
		}
		return 0;
	}

	static IPv6Address* Parse(Exception& ex,const char* address) {
		if (!address || *address==0 || !Net::InitializeNetwork(ex))
			return 0;
#if defined(_WIN32)
		if (String::ICompare(address, "localhost") == 0)
			address = LocalhostV6;
		struct addrinfo* pAI;
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_flags = AI_NUMERICHOST;
		int rc = getaddrinfo(address, NULL, &hints, &pAI);
		if (rc == 0) {
			IPv6Address* pResult = new IPv6Address(reinterpret_cast<struct sockaddr_in6*>(pAI->ai_addr)->sin6_addr, static_cast<int>(reinterpret_cast<struct sockaddr_in6*>(pAI->ai_addr)->sin6_scope_id));
			freeaddrinfo(pAI);
			return pResult;
		}
#else
		struct in6_addr ia;
		const char* scoped(strchr(address,'%'));
		if (scoped) {
			UInt32 scopeId(0);
			if (!(scopeId = if_nametoindex(scoped+1)))
				return 0;
			if(*address=='[')
				++address;
			auto result(inet_pton(AF_INET6,string(address,scoped - address).c_str(), &ia));
			if (result == 1)
                return new IPv6Address(ia, scopeId);
		} else {
			if (String::ICompare(address, "localhost") == 0)
				address = LocalhostV4;
			if(*address=='[') {
				if (inet_pton(AF_INET6,string(address+1,strlen(address)-2).c_str(), &ia) == 1)
					return new IPv6Address(ia);
			} else if (inet_pton(AF_INET6,address, &ia) == 1)
					return new IPv6Address(ia);
		}
#endif
		return 0;
	}

private:
	struct in6_addr		_addr;	
	UInt32				_scope;
	mutable string		_toString;
	mutable std::mutex	_mutex;
};


//
// IPAddress
//


class IPBroadcaster : public IPAddress {
public:
	IPBroadcaster() {
		struct in_addr ia;
		ia.s_addr = INADDR_NONE;
		set(ia);
	}
};

class IPWilcard : public IPAddress {
public:
	IPWilcard(Family family) {
		if (family == IPv6) {
			struct in6_addr	ia;	
			memset(&ia, 0, sizeof(ia));
			set(ia);
		} else {
			struct in_addr	ia;	
			memset(&ia, 0, sizeof(ia));
			set(ia);
		}
	}
};

class IPLoopback : public IPAddress {
public:
	IPLoopback(Family family) {
		Exception ex; // never ex
		set(ex, family == IPv6 ? LocalhostV6 : LocalhostV4);
	}
};

static IPWilcard	 _IPv4Wildcard(IPAddress::IPv4);
static IPWilcard	 _IPv6Wildcard(IPAddress::IPv6);

IPAddress::IPAddress(Family family) : _pIPAddress(family == IPv6 ? _IPv6Wildcard._pIPAddress : _IPv4Wildcard._pIPAddress) {
}

IPAddress::IPAddress(const IPAddress& other) : _pIPAddress(other._pIPAddress) {
}

IPAddress::IPAddress(const in_addr& addr) : _pIPAddress(new IPv4Address(addr)) {
}

IPAddress::IPAddress(const in6_addr& addr, UInt32 scope) : _pIPAddress(new IPv6Address(addr, scope)) {
}

void IPAddress::clear() {
	_pIPAddress = _pIPAddress->family() == IPv6 ? _IPv6Wildcard._pIPAddress : _IPv4Wildcard._pIPAddress;
}

IPAddress& IPAddress::set(const IPAddress& other) {
	_pIPAddress = other._pIPAddress;
	return *this;
}

IPAddress& IPAddress::set(const in_addr& addr) {
	_pIPAddress.reset(new IPv4Address(addr));
	return *this;
}

IPAddress& IPAddress::set(const in6_addr& addr, UInt32 scope) {
	_pIPAddress.reset(new IPv6Address(addr, scope));
	return *this;
}

bool IPAddress::set(Exception& ex, const char* address) {
	IPAddressCommon* pIPAddress = IPv4Address::Parse(ex, address);
	if (!pIPAddress)
		pIPAddress = IPv6Address::Parse(ex, address);
	if (!pIPAddress) {
		ex.set(Exception::NETIP, "Invalid ip ", address);
		return false;
	}
	_pIPAddress.reset(pIPAddress);
	return true;
}

bool IPAddress::setWithDNS(Exception& ex, const char* address) {
	if (set(ex, address))
		return true;
	HostEntry entry;
	if (!DNS::HostByName(ex, address, entry))
		return false;
	auto& addresses(entry.addresses());
	if (addresses.empty()) {
		ex.set(Exception::NETIP, "No ip found for address ", address);
		return false;
	}
	ex.set(Exception::NIL);
	// if we get both IPv4 and IPv6 addresses, prefer IPv4
	for (const IPAddress& address : addresses) {
		if (address.family() == IPAddress::IPv4) {
			set(address);
			return true;
		}
	}
	set(*addresses.begin());
	return true;
}

bool IPAddress::set(Exception& ex, const char* address, Family family) {
	IPAddressCommon* pIPAddress(NULL);
	if (family == IPv6)
		pIPAddress = IPv6Address::Parse(ex, address);
	else
		pIPAddress = IPv4Address::Parse(ex, address);
	if (!pIPAddress) {
		ex.set(Exception::NETIP, "Invalid ip ", address);
		return false;
	}
	_pIPAddress.reset(pIPAddress);
	return true;
}

bool IPAddress::mask(Exception& ex, const IPAddress& mask, const IPAddress& set) {
	ASSERT_RETURN(family() == IPAddress::IPv4 && mask.family() == IPAddress::IPv4 && set.family() == IPAddress::IPv4,false);
	_pIPAddress.reset(new IPv4Address(*(const in_addr*)_pIPAddress->addr(),*(const in_addr*)mask.addr(),*(const in_addr*)set.addr()));
	return true;
}

IPAddress::Family IPAddress::family() const {
	return _pIPAddress->family();
}
UInt32 IPAddress::scope() const {
	return _pIPAddress->scope();
}
const string& IPAddress::toString() const {
	return _pIPAddress->toString();
}
bool IPAddress::isWildcard() const {
	return _pIPAddress->isWildcard();
}
bool IPAddress::isBroadcast() const {
	return _pIPAddress->isBroadcast();
}
bool IPAddress::isAnyBroadcast() const {
	return _pIPAddress->isAnyBroadcast();
}
bool IPAddress::isLoopback() const {
	return _pIPAddress->isLoopback();
}
bool IPAddress::isMulticast() const {
	return _pIPAddress->isMulticast();
}
bool IPAddress::isLinkLocal() const {
	return _pIPAddress->isLinkLocal();
}
bool IPAddress::isSiteLocal() const {
	return _pIPAddress->isSiteLocal();
}
bool IPAddress::isIPv4Compatible() const {
	return _pIPAddress->isIPv4Compatible();
}
bool IPAddress::isIPv4Mapped() const {
	return _pIPAddress->isIPv4Mapped();
}
bool IPAddress::isWellKnownMC() const {
	return _pIPAddress->isWellKnownMC();
}
bool IPAddress::isNodeLocalMC() const {
	return _pIPAddress->isNodeLocalMC();
}
bool IPAddress::isLinkLocalMC() const {
	return _pIPAddress->isLinkLocalMC();
}
bool IPAddress::isSiteLocalMC() const {
	return _pIPAddress->isSiteLocalMC();
}
bool IPAddress::isOrgLocalMC() const {
	return _pIPAddress->isOrgLocalMC();
}
bool IPAddress::isGlobalMC() const {
	return _pIPAddress->isGlobalMC();
}
UInt8 IPAddress::prefixLength() const {
	return _pIPAddress->prefixLength();
}

bool IPAddress::operator == (const IPAddress& a) const {
	NET_SOCKLEN size = this->size();
	if (size != a.size())
		return false;
	const void* pAddr1(addr());
	const void* pAddr2(a.addr());
	return memcmp(pAddr1, pAddr2, size) == 0;
}

bool IPAddress::operator < (const IPAddress& a) const {
	NET_SOCKLEN size1(size());
	NET_SOCKLEN size2(a.size());
	if (size1 != size2)
		return size1<size2;
	const void* pAddr1(addr());
	const void* pAddr2(a.addr());
	return memcmp(pAddr1, pAddr2, size1) < 0;
}

const void* IPAddress::addr() const {
	return _pIPAddress->addr();
}

NET_SOCKLEN IPAddress::size() const {
	return _pIPAddress->size();
}

const IPAddress& IPAddress::Broadcast() {
	static IPBroadcaster IPBroadcast;
	return IPBroadcast;
}

const IPAddress& IPAddress::Wildcard(Family family) {
	return family == IPv6 ? _IPv6Wildcard : _IPv4Wildcard;
}

const IPAddress& IPAddress::Loopback(Family family) {
	static IPLoopback	 IPv4Loopback(IPAddress::IPv4);
	static IPLoopback	 IPv6Loopback(IPAddress::IPv6);
	return family == IPv6 ? IPv6Loopback : IPv4Loopback;
}

IPAddress::LocalAddresses::LocalAddresses() {

#if defined (WIN32)
		ULONG size = MAX_ADAPTERS_SIZE;
		Buffer buffer(MAX_ADAPTERS_SIZE);
		PIP_ADAPTER_ADDRESSES pAddresses = (IP_ADAPTER_ADDRESSES *)buffer.data(), adapt = NULL;
		PIP_ADAPTER_UNICAST_ADDRESS aip = NULL;

		if (GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, pAddresses, &size) != NO_ERROR) {
			emplace_back(Loopback(IPAddress::IPv4));
			return;
		}

		for (adapt = pAddresses; adapt; adapt = adapt->Next) {
			for (aip = adapt->FirstUnicastAddress; aip; aip = aip->Next) {

				if (aip->Address.lpSockaddr->sa_family == AF_INET6)
					emplace_back(reinterpret_cast<struct sockaddr_in6*>(aip->Address.lpSockaddr)->sin6_addr, reinterpret_cast<struct sockaddr_in6*>(aip->Address.lpSockaddr)->sin6_scope_id);
				else if (aip->Address.lpSockaddr->sa_family == AF_INET)
					emplace_back(reinterpret_cast<struct sockaddr_in*>(aip->Address.lpSockaddr)->sin_addr);
			}
		}
#else
		struct ifaddrs * ifAddrStruct = NULL;
		if (getifaddrs(&ifAddrStruct) == -1) {
			emplace_back(Loopback(IPAddress::IPv4));
			return;
		}

		for (struct ifaddrs * ifa = ifAddrStruct; ifa; ifa = ifa->ifa_next) {
			if (!ifa->ifa_addr) // address can be empty
				continue;

			if (ifa->ifa_addr->sa_family == AF_INET6)
				emplace_back(reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr)->sin6_addr, reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr)->sin6_scope_id);
			else if (ifa->ifa_addr->sa_family == AF_INET)
				emplace_back(reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr)->sin_addr);
		}
#endif
}

} // namespace Mona
