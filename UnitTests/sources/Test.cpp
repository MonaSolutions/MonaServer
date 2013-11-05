
#include "Test.h"
#include "Mona/Logs.h"

using namespace Mona;
using namespace std;

void PoolTest::runAll() {
	for(auto itTest : _mapTests) {
		itTest.second->TestFunction();
		NOTE(itTest.second->name(), " OK");
	}
}

void PoolTest::run(const string& mod) {
	auto itTest = _mapTests.equal_range(mod);
	if (itTest.first == itTest.second) 
		itTest = _mapTests.equal_range(mod + "Test");
	if (itTest.first == itTest.second) {
		ERROR("Module ",mod," does not exist.");
		return;
	}

	// Run all tests of the module
	for(auto it = itTest.first; it != itTest.second; it++) {
		it->second->TestFunction();
		NOTE(it->second->name(), " OK");
	}
}

void PoolTest::getListTests(vector<const string>& lTests) {

	for(auto itTest = _mapTests.begin(), end = _mapTests.end(); itTest != end ; itTest = _mapTests.upper_bound(itTest->first)) {
		lTests.push_back(itTest->first);
	}
}

PoolTest& PoolTest::PoolTestInstance () {
	static PoolTest ptest;
	return ptest;
}
