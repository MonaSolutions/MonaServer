
#pragma once

#include "Test.h"
#include "Mona/Exceptions.h"
#include "Mona/Logs.h"

// The fixture for testing class Foo.
class UtilTest : public Test {
protected:
	// You can remove any or all of the following functions if its body
	// is empty.

	UtilTest(const char * testName) : Test(testName) {
		// You can do set-up work for each test here.
	}

	virtual ~UtilTest() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

};

