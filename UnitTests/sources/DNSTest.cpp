
#include "Test.h"
#include "Mona/DNS.h"

using namespace std;
using namespace Mona;


HostEntry hostEntry;

ADD_TEST(DNSTest, HostByName) {
	Exception ex;

	DNS::HostByName(ex, "aliastest.appinf.com", hostEntry);
	EXPECT_TRUE(!ex)
	// different systems report different canonical names, unfortunately.
	EXPECT_TRUE(hostEntry.name() == "dnstest.appinf.com" || hostEntry.name() == "aliastest.appinf.com");

	EXPECT_TRUE(hostEntry.addresses().size() >= 1);
	EXPECT_TRUE(hostEntry.addresses().front().toString() == "1.2.3.4");

	DNS::HostByName(ex, "nohost.appinf.com", hostEntry);
	EXPECT_TRUE(ex); // must not to find the host
}

ADD_TEST(DNSTest, HostByAddress) {
	Exception ex;

	IPAddress ip;
	ip.set(ex, "80.122.195.86");
	EXPECT_TRUE(!ex)
	DNS::HostByAddress(ex, ip, hostEntry);
	EXPECT_TRUE(!ex)
	EXPECT_TRUE(hostEntry.name() == "mailhost.appinf.com");
	EXPECT_TRUE(hostEntry.aliases().empty());
	EXPECT_TRUE(hostEntry.addresses().size() >= 1);
	EXPECT_TRUE(hostEntry.addresses().front().toString() == "80.122.195.86");

	ip.set(ex, "10.0.244.253");
	EXPECT_TRUE(!ex)
	DNS::HostByAddress(ex, ip, hostEntry);
	EXPECT_TRUE(ex)
}
