
#pragma once

#include "Mona/Exceptions.h"
#include <memory>
#include <map>

/// \brief The fixture for testing class Foo.
class Test : virtual Mona::Object {
public:

	Test(const char * testName) {_name = testName;}
		/// \brief You can overload the constructor to do job before each test function

	virtual ~Test() {}
		/// \brief You can do clean-up work that doesn't throw exceptions here.

	virtual void TestFunction(){}
		/// \brief The test function to overload

	const std::string& name(){ return _name;}
		/// \brief accessor to the fullname of the test

private:
	std::string _name; /// fullname of the test
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
