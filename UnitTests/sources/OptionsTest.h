
#pragma once

#include "Test.h"
#include "Mona/Options.h"

// The fixture for testing class Foo.
class OptionsTest : public Test {
protected:
	// You can remove any or all of the following functions if its body
	// is empty.

	OptionsTest(const char * testName) : Test(testName) {
		// You can do set-up work for each test here.
	}

	virtual ~OptionsTest() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	bool AddOption(const std::string& fullName, const std::string& shortName, const std::string& description, 
			bool required = false, bool repeatable = false, const std::string& argName = "", bool argRequired = false);

	bool GetOption(const std::string& fullName);

	bool ProcessArg(char* arg, const std::function<void(Mona::Exception& ex, const std::string&, const std::string&)>& handler = nullptr);

	// Objects declared here can be used by all tests in the test case for Foo.
	Mona::Options opts;
};
