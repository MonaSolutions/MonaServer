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

#pragma once

#include "Mona/Exceptions.h"
#include "Mona/StopWatch.h"
#include <memory>
#include <map>

/// \brief The fixture for testing class Foo.
class Test : virtual Mona::Object {
public:

	Test(const char * testName) {_name = testName;}
		/// \brief You can overload the constructor to do job before each test function

	virtual ~Test() {}
		/// \brief You can do clean-up work that doesn't throw exceptions here.

	void run();

private:
	virtual void TestFunction() = 0;
		/// \brief The test function to overload


	std::string		_name; /// fullname of the test
	Mona::Stopwatch	_chrono;
};

/// \class Container of Test classes
class PoolTest : virtual Mona::Object {

public:	

	template<class TestClass>
    bool makeAndRegister(const char * className, const char * testName) { _mapTests.emplace(className, std::unique_ptr<TestClass>(new TestClass(testName))); return true; }
		/// \brief create the test and add it to the PoolTest

    void getListTests(std::vector<std::string>& lTests);
		/// \brief get a list of test module names

	void runAll();
		/// \brief Run all tests

	void run(const std::string& mod);
		/// \brief Run the test with 'mod' name

	static PoolTest& PoolTestInstance();
		/// \brief PoolTest Instance accessor

private:
    std::multimap<const std::string, std::unique_ptr<Test>> _mapTests;
		/// multimap of Test name to Tests functions
			
	PoolTest(){}
		/// \brief PoolTest Constructor

	virtual ~PoolTest(){}
		/// \brief destructor of PoolTest

};

/// Macro for assert true function
#define CHECK(CONDITION) FATAL_ASSERT(CONDITION)

/// Macro for adding new tests in a Test cpp
#define ADD_TEST(CLASSNAME, TESTNAME) class CLASSNAME ## TESTNAME : public Test { \
public: \
	CLASSNAME ## TESTNAME(const char * testName) : Test(testName) {}\
	virtual ~CLASSNAME ## TESTNAME() {}\
	virtual void TestFunction();\
private:\
	static const bool _TestCreated;\
};\
const bool CLASSNAME ## TESTNAME::_TestCreated = PoolTest::PoolTestInstance().makeAndRegister<CLASSNAME ## TESTNAME>(#CLASSNAME, #CLASSNAME "::" #TESTNAME);\
void CLASSNAME ## TESTNAME::TestFunction()
