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
#include "Mona/DNS.h"

using namespace std;
using namespace Mona;


ADD_TEST(DNSTest, HostByName) {
	Exception ex;
	HostEntry hostEntry;

	DNS::HostByName(ex, "aliastest.appinf.com", hostEntry);
	CHECK(!ex)
	// different systems report different canonical names, unfortunately.
	CHECK(hostEntry.name() == "dnstest.appinf.com" || hostEntry.name() == "aliastest.appinf.com");

	CHECK(hostEntry.addresses().size() >= 1);
	CHECK((*hostEntry.addresses().begin()).toString() == "1.2.3.4");

	DNS::HostByName(ex, "nohost.appinf.com", hostEntry);
	CHECK(ex); // must not to find the host

	HostEntry hostEntry2;
	Exception ex2;
	DNS::HostByName(ex2, "monaserver.ovh", hostEntry2);
	CHECK(!ex2)
	DNS::HostByName(ex2, "monaserver.ovh", hostEntry2);
	CHECK(!ex2)
	CHECK(hostEntry2.addresses().size() == 1)
}

ADD_TEST(DNSTest, HostByAddress) {
	Exception ex;
	HostEntry hostEntry;

	IPAddress ip;
	ip.set(ex, "80.122.195.86");
	CHECK(!ex)
	DNS::HostByAddress(ex, ip, hostEntry);
	CHECK(!ex)
	CHECK(hostEntry.name() == "mailhost.appinf.com");
	CHECK(hostEntry.aliases().empty());
	CHECK(hostEntry.addresses().size() >= 1);
	CHECK((*hostEntry.addresses().begin()).toString() == "80.122.195.86");

	ip.set(ex, "10.0.244.253");
	CHECK(!ex)
	DNS::HostByAddress(ex, ip, hostEntry);
	CHECK(ex)
}
