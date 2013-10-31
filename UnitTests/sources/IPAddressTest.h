
#pragma once

#include "Test.h"
#include "Mona/IPAddress.h"

// The fixture for testing class Foo.
class IPAddressTest : public Test {
protected:
	// You can remove any or all of the following functions if its body
	// is empty.

	IPAddressTest(const char * testName) : Test(testName) {
		// You can do set-up work for each test here.
	}

	virtual ~IPAddressTest() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	static Mona::IPAddress	_IpAddress;
};
