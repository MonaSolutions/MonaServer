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
#include "Mona/Net.h"
#include <memory>


namespace Mona {

class IPAddressCommon;

/// This class represents an internet (IP) host
/// address. The address can belong either to the
/// IPv4 or the IPv6 address family.
///
/// Relational operators (==, !=, <, <=, >, >=) are
/// supported. However, you must not interpret any
/// special meaning into the result of these 
/// operations, other than that the results are
/// consistent.
///
/// Especially, an IPv4 address is never equal to
/// an IPv6 address, even if the IPv6 address is
/// IPv4 compatible and the addresses are the same.
///
/// IPv6 addresses are supported only if the target platform
/// supports IPv6.

class IPAddress : virtual Object {
public:
	enum Family {
		IPv4=1,
		IPv6
	};
	
	// Creates a wildcard (zero) IPv4 IPAddress.
	IPAddress(Family family=IPv4);
	IPAddress(const IPAddress& other);

	// Set an IPAddress from a native internet address. A pointer to a in_addr or a in6_addr structure may be  passed. Additionally, for an IPv6 address, a scope ID may be specified.
	bool copy(Exception& ex, const void* addr, UInt32 scope=0);

	// Set an IPAddress from the string containing an IP address in presentation format (dotted decimal for IPv4, hex string for IPv6).
	bool set(Exception& ex, const std::string& addr);

	// Set an IPAddress from the string containing an IP address in presentation format (dotted decimal for IPv4, hex string for IPv6).
	bool set(Exception& ex, const std::string& addr, Family family);

	// Masks the IP address using the given netmask, which is usually a IPv4 subnet mask (Only supported for IPv4 addresses)
	// The new address is (address & mask)
	void mask(Exception& ex, const IPAddress& mask);
	// Masks the IP address using the given netmask, which is usually a IPv4 subnet mask (Only supported for IPv4 addresses)
	// The new address is (address & mask) | (set & ~mask)
	void mask(Exception& ex, const IPAddress& mask, const IPAddress& set);



	// Returns the address family (IPv4 or IPv6) of the address
	Family family() const;
	
	// Returns the IPv6 scope identifier of the address. Returns 0 if the address is an IPv4 address
	Poco::UInt32 scope() const;

	// Returns a string containing a representation of the address in presentation format
	const std::string& toString() const;
	
	// Returns true iff the address is a wildcard (all zero) address
	bool isWildcard() const;
		
	// Returns true iff the address is a broadcast address (Only IPv4 addresses can be broadcast addresse)
	bool isBroadcast() const;
	
	// Returns true iff the address is a loopback address
	// 127.0.0.1 for IPv4, ::1 for IPv6
	bool isLoopback() const;
	
	// Returns true iff the address is a multicast address
	// 224.0.0.0 to 239.255.255.255 for IPv4, FFxx:x:x:x:x:x:x:x range for IPv6
	bool isMulticast() const;
		
	// Returns true iff the address is a unicast address (if it is neither a wildcard, broadcast or multicast address)
	bool isUnicast() const;
		
	// Returns true iff the address is a link local unicast address
	// 169.254.0.0/16 range for IPv4, 1111 1110 10 as the first 10 bits followed by 54 zeros for IPv6
	bool isLinkLocal() const;
		
	// Returns true iff the address is a site local unicast address
	// 10.0.0.0/24, 192.168.0.0/16 or 172.16.0.0 to 172.31.255.255 ranges for IPv4, 1111 1110 11 as the first 10 bits, followed by 38 zeros for IPv6
	bool isSiteLocal() const;
		
	// Returns true iff the address is IPv4 compatible (for IPv6 the address must be in the ::x:x range)
	bool isIPv4Compatible() const;

	// Returns true iff the address is an IPv4 mapped IPv6 address (For IPv6, the address must be in the ::FFFF:x:x range)
	bool isIPv4Mapped() const;

	// Returns true iff the address is a well-known multicast address
	// 224.0.0.0/8 range for IPv4, FF0x:x:x:x:x:x:x:x range for IPv6
	bool isWellKnownMC() const;
	
	// Returns true iff the address is a node-local multicast address
	// Always false for IPv4, node-local multicast addresses are in the FFx1:x:x:x:x:x:x:x range for IPv6
	bool isNodeLocalMC() const;

	// Returns true iff the address is a link-local multicast address
	// 224.0.0.0/24 range for IPv4, FFx2:x:x:x:x:x:x:x range for IPv6
	bool isLinkLocalMC() const;

	// Returns true iff the address is a site-local multicast address
	// 239.255.0.0/16 range for IPv4, FFx5:x:x:x:x:x:x:x for IPv6
	bool isSiteLocalMC() const;

	// Returns true iff the address is a organization-local multicast address
	// 239.192.0.0/16 range for IPv4, FFx8:x:x:x:x:x:x:x range for IPv6
	bool isOrgLocalMC() const;

	// Returns true iff the address is a global multicast address
	// 224.0.1.0 to 238.255.255.255 range for IPv4, FFxF:x:x:x:x:x:x:x range for IPv6
	bool isGlobalMC() const;
	
	bool operator == (const IPAddress& addr) const;	
	bool operator != (const IPAddress& addr) const;
	bool operator <  (const IPAddress& addr) const;
	bool operator <= (const IPAddress& addr) const;
	bool operator >  (const IPAddress& addr) const;
	bool operator >= (const IPAddress& addr) const;
	
	// Returns the internal address structure
	const void* addr() const;

	// Returns a wildcard IPv4 or IPv6 address (0.0.0.0)
	static const IPAddress& Wildcard(Family family = IPv4) { return family == IPv6 ? _IPv6Wildcard : _IPv4Wildcard; }
	// Returns a broadcast IPv4 address (255.255.255.255)
	static const IPAddress& Broadcast();

	enum {
		MAX_ADDRESS_LENGTH = sizeof(struct in6_addr)
		/// Maximum length in bytes of a socket address.
	};

private:

	std::shared_ptr<IPAddressCommon>	_pIPAddress;

	static IPAddress					_IPv4Wildcard;
	static IPAddress					_IPv6Wildcard;
};



} // namespace Mona
