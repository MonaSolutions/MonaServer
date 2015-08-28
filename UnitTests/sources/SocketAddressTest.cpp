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
#include "Mona/SocketAddress.h"
#include "Mona/Logs.h"

using namespace std;
using namespace Mona;

static string _Buffer;

ADD_TEST(SocketAddressTest, Behavior) {
	
	SocketAddress sa;
	Exception ex;
	

	CHECK(!sa);
	CHECK(sa.host().isWildcard());
	CHECK(sa.port() == 0);

	sa.set(ex,"192.168.1.100", 100);
	CHECK(sa.host().toString() == "192.168.1.100");
	CHECK(sa.port() == 100);

	sa.set(ex, "192.168.1.100", "100");
	CHECK(sa.host().toString() == "192.168.1.100");
	CHECK(sa.port() == 100);


	sa.set(ex, "192.168.1.100", "ftp");
	CHECK(sa.host().toString() == "192.168.1.100");
	CHECK(sa.port() == 21);

	CHECK(!ex);

	sa.set(ex,"192.168.1.100", "f00bar");
	CHECK(ex);

	ex.set(Exception::NIL, "");

	DEBUG_CHECK(sa.setWithDNS(ex,"localhost", 80)); // TODO see if we keep this address
	DEBUG_CHECK(sa.host().toString() == "127.0.0.1");
	DEBUG_CHECK(sa.port() == 80);
	DEBUG_CHECK(!ex);

	sa.set(ex,"192.168.2.260", 80);

	CHECK(ex);
	ex.set(Exception::NIL, "");

	sa.set(ex, "192.168.2.120", "80000");
	
	CHECK(ex);
	ex.set(Exception::NIL, "");

	sa.set(ex,_Buffer.assign("192.168.2.120:88"));
	CHECK(sa.host().toString() == "192.168.2.120");
	CHECK(sa.port() == 88);

	sa.set(ex, _Buffer.assign("[192.168.2.120]:88"));
	CHECK(ex);
	ex.set(Exception::NIL, "");
	sa.set(ex, "[192.168.2.260]");
	CHECK(ex);
	ex.set(Exception::NIL, "");

	sa.set(ex,_Buffer.assign("[192.168.2.260:88"));
	CHECK(ex);
	ex.set(Exception::NIL, "");
	
	sa.set(ex,"192.168.1.100", 100);
	SocketAddress sa2;
	sa2.set(ex,_Buffer.assign("192.168.1.100:100"));
	CHECK(sa == sa2);

	sa.set(ex,"192.168.1.101", "99");
	CHECK(sa2 < sa);

	sa2.set(ex, "192.168.1.101", "102");
	CHECK(sa < sa2);

	CHECK(!ex);
}

ADD_TEST(SocketAddressTest, ToString) {
	// toString performance (for loop test)
	SocketAddress sa;
	sa.toString();
}

ADD_TEST(SocketAddressTest, ParsePerformance) {
	// Parse performance (for loop test)
	SocketAddress sa;
	Exception ex;
	sa.set(ex,"192.168.1.100",100);
}

ADD_TEST(SocketAddressTest, ComparaisonPerformance) {
	// Comparaison performance (for loop test)
	SocketAddress sa;
	SocketAddress sa2(sa);
	if (sa < sa2)
		CHECK(false)
}

ADD_TEST(SocketAddressTest, Behavior6) {
	
	SocketAddress sa;
	Exception ex;

	CHECK(!sa);
	CHECK(sa.host().isWildcard());
	CHECK(sa.port() == 0);

	sa.set(ex,"1080::8:600:200A:425C", 100);
	CHECK(sa.host().toString() == "1080::8:600:200a:425c");
	CHECK(sa.port() == 100);

	sa.set(ex, "1080::8:600:200A:425C", "100");
	CHECK(sa.host().toString() == "1080::8:600:200a:425c");
	CHECK(sa.port() == 100);


	sa.set(ex, "1080::8:600:200A:425C", "ftp");
	CHECK(sa.host().toString() == "1080::8:600:200a:425c");
	CHECK(sa.port() == 21);

	sa.set(ex, "1080::0001", "65535");
	CHECK(sa.host().toString() == "1080::1");
	CHECK(sa.port() == 65535);

	CHECK(!ex);

	sa.set(ex,"1080::8:600:200A:425C", "f00bar");
	CHECK(ex);

	ex.set(Exception::NIL, "");

	sa.set(ex,"1080::8:600:200A:FFFFF", 80);

	CHECK(ex);
	ex.set(Exception::NIL, "");

	sa.set(ex, "1080::8:600:200A:425C", "80000");
	
	CHECK(ex);
	ex.set(Exception::NIL, "");

	sa.set(ex,_Buffer.assign("[1080::8:600:200A:425C]:88"));
	CHECK(sa.host().toString() == "1080::8:600:200a:425c");
	CHECK(sa.port() == 88);

	CHECK(!ex);

	sa.set(ex, _Buffer.assign("[1080::8:600:200A:425C]"));
	CHECK(ex);
	ex.set(Exception::NIL, "");

	sa.set(ex,_Buffer.assign("[1080::8:600:200A:425C:88]"));
	CHECK(ex);
	ex.set(Exception::NIL, "");
	
	sa.set(ex,"1080::8:600:200A:425C", 100);
	SocketAddress sa2;
	sa2.set(ex,_Buffer.assign("[1080::8:600:200A:425C]:100"));
	CHECK(sa == sa2);

	sa.set(ex,"1080::8:600:200A:425D", "99");
	CHECK(sa2 < sa);

	sa2.set(ex, "1080::8:600:200A:425D", "102");
	CHECK(sa < sa2);

	sa.set(ex,"1080::0001", "100");
	sa2.set(ex, "1080::1", "100");
	CHECK(sa == sa2);

	CHECK(!ex);
}

const UInt32 MAX_UINT32 = 65536;

ADD_TEST(SocketAddressTest, Comparisons6) {
	string address, addressAndPort;
	Exception ex;
	SocketAddress sAddress1, sAddress2;

	// Test of all 0:0:0:0:0:0:0:X formats
	for (UInt32 firstPart = 1; firstPart < MAX_UINT32; firstPart++) {
		String::Format(address, "0:0:0:0:0:0:0:", Format<UInt32>("%.X", firstPart));
		String::Format(addressAndPort, "[", address, "]:1234");

		sAddress1.set(ex, addressAndPort);
		sAddress2.set(ex, address, 1234);

		CHECK(!ex);
		CHECK(sAddress1==sAddress2);
	}
}
