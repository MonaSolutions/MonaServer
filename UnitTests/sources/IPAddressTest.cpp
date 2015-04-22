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

#include "Test.h"
#include "Mona/IPAddress.h"
#include "Mona/Exceptions.h"

using namespace Mona;
using namespace std;

static IPAddress	_IpAddress;

ADD_TEST(IPAddressTest, Parse) {
	Exception ex;
	CHECK(_IpAddress.set(ex, "192.168.1.120") && !ex);
	CHECK(!_IpAddress.set(ex, "192.168.1.280") && ex);
}

ADD_TEST(IPAddressTest, StringConv) {
	
	Exception ex;
	CHECK(_IpAddress.set(ex, "127.0.0.1"));
	CHECK(!ex);
	CHECK(_IpAddress.family() == IPAddress::IPv4);
	CHECK(_IpAddress.toString() == "127.0.0.1");
	
	CHECK(_IpAddress.set(ex, "192.168.1.120"));
	CHECK(!ex);
	CHECK(_IpAddress.family() == IPAddress::IPv4);
	CHECK(_IpAddress.toString() == "192.168.1.120");
	
	CHECK(_IpAddress.set(ex, "255.255.255.255"));
	CHECK(!ex);
	CHECK(_IpAddress.family() == IPAddress::IPv4);
	CHECK(_IpAddress.toString() == "255.255.255.255");

	CHECK(_IpAddress.set(ex, "0.0.0.0"));
	CHECK(!ex);
	CHECK(_IpAddress.family() == IPAddress::IPv4);
	CHECK(_IpAddress.toString() == "0.0.0.0");
}


ADD_TEST(IPAddressTest, StringConv6) {

	Exception ex;
	CHECK(_IpAddress.set(ex, "1080:0:0:0:8:600:200A:425C"));
	CHECK(!ex);
	CHECK(_IpAddress.family() == IPAddress::IPv6);
	CHECK(_IpAddress.toString() == "1080::8:600:200a:425c");
	
	CHECK(_IpAddress.set(ex, "1080::8:600:200a:425c"));
	CHECK(!ex);
	CHECK(_IpAddress.family() == IPAddress::IPv6);
	CHECK(_IpAddress.toString() == "1080::8:600:200a:425c");
	
	CHECK(_IpAddress.set(ex, "::192.168.1.120"));
	CHECK(!ex);
	CHECK(_IpAddress.family() == IPAddress::IPv6);
	CHECK(_IpAddress.toString() == "::192.168.1.120");

	CHECK(_IpAddress.set(ex, "::FFFF:192.168.1.120"));
	CHECK(!ex);
	CHECK(_IpAddress.family() == IPAddress::IPv6);
	CHECK(_IpAddress.toString() == "::FFFF:192.168.1.120");
}

ADD_TEST(IPAddressTest, Classification) {

	Exception ex;
	CHECK(_IpAddress.set(ex, "0.0.0.0")); // wildcard
	CHECK(!ex);
	CHECK(_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==0);
		
	CHECK(_IpAddress.set(ex, "255.255.255.255")); // broadcast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(_IpAddress.isBroadcast());
	CHECK(_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==32);

	CHECK(_IpAddress.set(ex, "192.168.255.255")); // sub broadcast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==32);
	
	CHECK(_IpAddress.set(ex, "127.0.0.1")); // loopback
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==32);

	CHECK(_IpAddress.set(ex, "80.122.195.86")); // unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==31);

	CHECK(_IpAddress.set(ex, "169.254.1.20")); // link local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(_IpAddress.isUnicast());
	CHECK(_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==30);

	CHECK(_IpAddress.set(ex, "192.168.1.120")); // site local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==29);

	CHECK(_IpAddress.set(ex, "10.0.0.138")); // site local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==31);

	CHECK(_IpAddress.set(ex, "172.18.1.200")); // site local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==29);
}

ADD_TEST(IPAddressTest, MCClassification) {

	Exception ex;
	CHECK(_IpAddress.set(ex, "224.0.0.100")); // well-known multicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(_IpAddress.isLinkLocalMC()); // well known are in the range of link local
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==30);

	CHECK(_IpAddress.set(ex, "224.1.0.100")); // link local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(_IpAddress.isGlobalMC()); // link local fall in the range of global
	CHECK(_IpAddress.prefixLength()==30);

	CHECK(_IpAddress.set(ex, "239.255.0.100")); // site local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==30);

	CHECK(_IpAddress.set(ex, "239.192.0.100")); // org local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==30);

	CHECK(_IpAddress.set(ex, "224.2.127.254")); // global unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(_IpAddress.isLinkLocalMC()); // link local fall in the range of global
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==31);
}

ADD_TEST(IPAddressTest, Classification6) {

	Exception ex;
	CHECK(_IpAddress.set(ex, "::")); // wildcard
	CHECK(!ex);
	CHECK(_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==0);
		
	CHECK(_IpAddress.set(ex, "::1")); // loopback
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==128);

	CHECK(_IpAddress.set(ex, "2001:0db8:85a3:0000:0000:8a2e:0370:7334")); // unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==126);

	CHECK(_IpAddress.set(ex, "fe80::21f:5bff:fec6:6707")); // link local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(_IpAddress.isUnicast());
	CHECK(_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==128);

	CHECK(_IpAddress.set(ex, "fe80::12")); // link local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(_IpAddress.isUnicast());
	CHECK(_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==127);

	CHECK(_IpAddress.set(ex, "fec0::21f:5bff:fec6:6707")); // site local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(!_IpAddress.isMulticast());
	CHECK(_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==128);
}


ADD_TEST(IPAddressTest, MCClassification6) {

	Exception ex;
	CHECK(_IpAddress.set(ex, "ff02:0:0:0:0:0:0:c")); // well-known link-local multicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(_IpAddress.isLinkLocalMC()); 
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==126);

	CHECK(_IpAddress.set(ex, "FF01:0:0:0:0:0:0:FB")); // node-local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(_IpAddress.isWellKnownMC());
	CHECK(_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC()); 
	CHECK(_IpAddress.prefixLength()==128);

	CHECK(_IpAddress.set(ex, "FF05:0:0:0:0:0:0:FB")); // site local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==128);

	CHECK(_IpAddress.set(ex, "FF18:0:0:0:0:0:0:FB")); // org local unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC());
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(_IpAddress.isOrgLocalMC());
	CHECK(!_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==128);

	CHECK(_IpAddress.set(ex, "FF1F:0:0:0:0:0:0:FB")); // global unicast
	CHECK(!ex);
	CHECK(!_IpAddress.isWildcard());
	CHECK(!_IpAddress.isBroadcast());
	CHECK(!_IpAddress.isAnyBroadcast());
	CHECK(!_IpAddress.isLoopback());
	CHECK(_IpAddress.isMulticast());
	CHECK(!_IpAddress.isUnicast());
	CHECK(!_IpAddress.isLinkLocal());
	CHECK(!_IpAddress.isSiteLocal());
	CHECK(!_IpAddress.isWellKnownMC());
	CHECK(!_IpAddress.isNodeLocalMC());
	CHECK(!_IpAddress.isLinkLocalMC()); 
	CHECK(!_IpAddress.isSiteLocalMC());
	CHECK(!_IpAddress.isOrgLocalMC());
	CHECK(_IpAddress.isGlobalMC());
	CHECK(_IpAddress.prefixLength()==128);
}


ADD_TEST(IPAddressTest, Relationals) {

	Exception ex;
	IPAddress ip1;
	CHECK(ip1.set(ex, "192.168.1.120"));
	CHECK(!ex);
	IPAddress ip2(ip1);
	IPAddress ip3;
	IPAddress ip4;
	CHECK(ip4.set(ex, "10.0.0.138"));
	CHECK(!ex);
	
	CHECK(ip1 != ip4);
	CHECK(ip1 == ip2);
	CHECK(!(ip1 != ip2));
	CHECK(!(ip1 == ip4));
	CHECK(ip1 > ip4);
	CHECK(ip1 >= ip4);
	CHECK(ip4 < ip1);
	CHECK(ip4 <= ip1);
	CHECK(!(ip1 < ip4));
	CHECK(!(ip1 <= ip4));
	CHECK(!(ip4 > ip1));
	CHECK(!(ip4 >= ip1));
}


ADD_TEST(IPAddressTest, Relationals6) {

	Exception ex;
	IPAddress ip1;
	CHECK(ip1.set(ex, "fe80::21f:5bff:fec6:6707"));
	CHECK(!ex);
	IPAddress ip2(ip1);
	IPAddress ip3;
	IPAddress ip4;
	CHECK(ip4.set(ex, "2001:0db8:0000:85a3:0000:0000:ac1f:8001"));
	CHECK(!ex);
	IPAddress ip5;
	CHECK(ip5.set(ex, "2001:0db8:0000:85a3::ac1f:8001"));
	CHECK(!ex);
	IPAddress ip6;
	CHECK(ip6.set(ex, "2001:0db8:0:85a3:0:0:ac1f:8001"));
	CHECK(!ex);
	IPAddress ip7;
	CHECK(ip7.set(ex, "2001:0db8:0:85a3::ac1f:8001"));
	CHECK(!ex);

	CHECK(ip4 == ip5);
	CHECK(ip4 == ip6);
	CHECK(ip4 == ip7);
	CHECK(ip5 == ip6);
	CHECK(ip5 == ip7);
	CHECK(ip6 == ip7);
	
	CHECK(ip1 != ip4);
	CHECK(ip1 == ip2);
	CHECK(!(ip1 != ip2));
	CHECK(!(ip1 == ip4));
	CHECK(ip1 > ip4);
	CHECK(ip1 >= ip4);
	CHECK(ip4 < ip1);
	CHECK(ip4 <= ip1);
	CHECK(!(ip1 < ip4));
	CHECK(!(ip1 <= ip4));
	CHECK(!(ip4 > ip1));
	CHECK(!(ip4 >= ip1));
}
