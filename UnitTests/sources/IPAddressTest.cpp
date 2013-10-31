
#include "IPAddressTest.h"
#include "Mona/Exceptions.h"

using namespace Mona;
using namespace std;

IPAddress	IPAddressTest::_IpAddress;

ADD_TEST(IPAddressTest, StringConv) {
	
	Exception ex;
	EXPECT_TRUE(_IpAddress.set(ex, "127.0.0.1"));
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(_IpAddress.family() == IPAddress::IPv4);
	EXPECT_TRUE(_IpAddress.toString() == "127.0.0.1");
	
	EXPECT_TRUE(_IpAddress.set(ex, "192.168.1.120"));
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(_IpAddress.family() == IPAddress::IPv4);
	EXPECT_TRUE(_IpAddress.toString() == "192.168.1.120");
	
	EXPECT_TRUE(_IpAddress.set(ex, "255.255.255.255"));
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(_IpAddress.family() == IPAddress::IPv4);
	EXPECT_TRUE(_IpAddress.toString() == "255.255.255.255");

	EXPECT_TRUE(_IpAddress.set(ex, "0.0.0.0"));
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(_IpAddress.family() == IPAddress::IPv4);
	EXPECT_TRUE(_IpAddress.toString() == "0.0.0.0");
}


ADD_TEST(IPAddressTest, StringConv6) {

	Exception ex;
	EXPECT_TRUE(_IpAddress.set(ex, "1080:0:0:0:8:600:200A:425C"));
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(_IpAddress.family() == IPAddress::IPv6);
	EXPECT_TRUE(_IpAddress.toString() == "1080::8:600:200A:425C");
	
	EXPECT_TRUE(_IpAddress.set(ex, "1080::8:600:200A:425C"));
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(_IpAddress.family() == IPAddress::IPv6);
	EXPECT_TRUE(_IpAddress.toString() == "1080::8:600:200A:425C");
	
	EXPECT_TRUE(_IpAddress.set(ex, "::192.168.1.120"));
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(_IpAddress.family() == IPAddress::IPv6);
	EXPECT_TRUE(_IpAddress.toString() == "::192.168.1.120");

	EXPECT_TRUE(_IpAddress.set(ex, "::FFFF:192.168.1.120"));
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(_IpAddress.family() == IPAddress::IPv6);
	EXPECT_TRUE(_IpAddress.toString() == "::FFFF:192.168.1.120");
}

ADD_TEST(IPAddressTest, Parse) {

	Exception ex;
	EXPECT_TRUE(_IpAddress.set(ex, "192.168.1.120"));
	EXPECT_TRUE(!ex);

	EXPECT_TRUE(!_IpAddress.set(ex, "192.168.1.280"));
	EXPECT_TRUE(ex);
}

ADD_TEST(IPAddressTest, Classification) {

	Exception ex;
	EXPECT_TRUE(_IpAddress.set(ex, "0.0.0.0")); // wildcard
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());
		
	EXPECT_TRUE(_IpAddress.set(ex, "255.255.255.255")); // broadcast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());
	
	EXPECT_TRUE(_IpAddress.set(ex, "127.0.0.1")); // loopback
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "80.122.195.86")); // unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "169.254.1.20")); // link local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(_IpAddress.isUnicast());
	EXPECT_TRUE(_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "192.168.1.120")); // site local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "10.0.0.138")); // site local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "172.18.1.200")); // site local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());
}

ADD_TEST(IPAddressTest, MCClassification) {

	Exception ex;
	EXPECT_TRUE(_IpAddress.set(ex, "224.0.0.100")); // well-known multicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(_IpAddress.isLinkLocalMC()); // well known are in the range of link local
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "224.1.0.100")); // link local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(_IpAddress.isGlobalMC()); // link local fall in the range of global

	EXPECT_TRUE(_IpAddress.set(ex, "239.255.0.100")); // site local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "239.192.0.100")); // org local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "224.2.127.254")); // global unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(_IpAddress.isLinkLocalMC()); // link local fall in the range of global
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(_IpAddress.isGlobalMC());
}

ADD_TEST(IPAddressTest, Classification6) {

	Exception ex;
	EXPECT_TRUE(_IpAddress.set(ex, "::")); // wildcard
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());
		
	EXPECT_TRUE(_IpAddress.set(ex, "::1")); // loopback
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "2001:0db8:85a3:0000:0000:8a2e:0370:7334")); // unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "fe80::21f:5bff:fec6:6707")); // link local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(_IpAddress.isUnicast());
	EXPECT_TRUE(_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "fe80::12")); // link local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(_IpAddress.isUnicast());
	EXPECT_TRUE(_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "fec0::21f:5bff:fec6:6707")); // site local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(!_IpAddress.isMulticast());
	EXPECT_TRUE(_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());
}


ADD_TEST(IPAddressTest, MCClassification6) {

	Exception ex;
	EXPECT_TRUE(_IpAddress.set(ex, "ff02:0:0:0:0:0:0:c")); // well-known link-local multicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(_IpAddress.isLinkLocalMC()); 
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "FF01:0:0:0:0:0:0:FB")); // node-local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(_IpAddress.isWellKnownMC());
	EXPECT_TRUE(_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC()); 

	EXPECT_TRUE(_IpAddress.set(ex, "FF05:0:0:0:0:0:0:FB")); // site local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "FF18:0:0:0:0:0:0:FB")); // org local unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC());
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(!_IpAddress.isGlobalMC());

	EXPECT_TRUE(_IpAddress.set(ex, "FF1F:0:0:0:0:0:0:FB")); // global unicast
	EXPECT_TRUE(!ex);
	EXPECT_TRUE(!_IpAddress.isWildcard());
	EXPECT_TRUE(!_IpAddress.isBroadcast());
	EXPECT_TRUE(!_IpAddress.isLoopback());
	EXPECT_TRUE(_IpAddress.isMulticast());
	EXPECT_TRUE(!_IpAddress.isUnicast());
	EXPECT_TRUE(!_IpAddress.isLinkLocal());
	EXPECT_TRUE(!_IpAddress.isSiteLocal());
	EXPECT_TRUE(!_IpAddress.isWellKnownMC());
	EXPECT_TRUE(!_IpAddress.isNodeLocalMC());
	EXPECT_TRUE(!_IpAddress.isLinkLocalMC()); 
	EXPECT_TRUE(!_IpAddress.isSiteLocalMC());
	EXPECT_TRUE(!_IpAddress.isOrgLocalMC());
	EXPECT_TRUE(_IpAddress.isGlobalMC());
}


ADD_TEST(IPAddressTest, Relationals) {

	Exception ex;
	IPAddress ip1;
	EXPECT_TRUE(ip1.set(ex, "192.168.1.120"));
	EXPECT_TRUE(!ex);
	IPAddress ip2(ip1);
	IPAddress ip3;
	IPAddress ip4;
	EXPECT_TRUE(ip4.set(ex, "10.0.0.138"));
	EXPECT_TRUE(!ex);
	
	EXPECT_TRUE(ip1 != ip4);
	EXPECT_TRUE(ip1 == ip2);
	EXPECT_TRUE(!(ip1 != ip2));
	EXPECT_TRUE(!(ip1 == ip4));
	EXPECT_TRUE(ip1 > ip4);
	EXPECT_TRUE(ip1 >= ip4);
	EXPECT_TRUE(ip4 < ip1);
	EXPECT_TRUE(ip4 <= ip1);
	EXPECT_TRUE(!(ip1 < ip4));
	EXPECT_TRUE(!(ip1 <= ip4));
	EXPECT_TRUE(!(ip4 > ip1));
	EXPECT_TRUE(!(ip4 >= ip1));
}
