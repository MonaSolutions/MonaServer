
#pragma once

#include "Test.h"
#include "Mona/BinaryWriter.h"
#include "Mona/BinaryReader.h"

// The fixture for testing class Foo.
class BinaryReaderWriterTest : public Test {
public:
	// You can remove any or all of the following functions if its body
	// is empty.

	BinaryReaderWriterTest(const char * testName) : Test(testName) {
		// You can do set-up work for each test here.
	}

	virtual ~BinaryReaderWriterTest() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	// Objects declared here can be used by all tests in the test case for Foo.

	void write(Mona::BinaryWriter& writer);
	void read(Mona::BinaryReader& reader);
};

