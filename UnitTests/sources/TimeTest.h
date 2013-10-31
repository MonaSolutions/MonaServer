
#pragma once

#include "Test.h"

// The fixture for testing class Foo.
class TimeTest : public Test {
protected:
	// You can remove any or all of the following functions if its body
	// is empty.

	TimeTest(const char * testName) : Test(testName) {
		// You can do set-up work for each test here.
	}

	virtual ~TimeTest() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	// Objects declared here can be used by all tests in the test case for Foo.
};

