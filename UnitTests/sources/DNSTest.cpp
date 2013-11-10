
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
	CHECK(hostEntry.addresses().front().toString() == "1.2.3.4");

	DNS::HostByName(ex, "nohost.appinf.com", hostEntry);
	CHECK(ex); // must not to find the host
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
	CHECK(hostEntry.addresses().front().toString() == "80.122.195.86");

	ip.set(ex, "10.0.244.253");
	CHECK(!ex)
	DNS::HostByAddress(ex, ip, hostEntry);
	CHECK(ex)
}
