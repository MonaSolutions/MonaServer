
#pragma once

#include "gtest/gtest.h"
#include "Mona/Options.h"

// The fixture for testing class Foo.
class OptionsTest : public ::testing::Test {
protected:
	// You can remove any or all of the following functions if its body
	// is empty.

	OptionsTest() {
		// You can do set-up work for each test here.
	}

	virtual ~OptionsTest() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp() {
		// Code here will be called immediately after the constructor (right
		// before each test).
	}

	virtual void TearDown() {
		// Code here will be called immediately after each test (right
		// before the destructor).
	}

	void removeAllOptions();

	static ::testing::AssertionResult AddOption(const std::string& fullName, const std::string& shortName, const std::string& description, 
															 bool required = false, bool repeatable = false, const std::string& argName = "", bool argRequired = false);

	static ::testing::AssertionResult GetOption(const std::string& fullName);

	// Objects declared here can be used by all tests in the test case for Foo.
	static Mona::Options opts;
};

