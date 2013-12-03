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

#include "Test.h"
#include "Mona/Logs.h"

using namespace Mona;
using namespace std;


void Test::run() {
	_chrono.restart();
	TestFunction();
	_chrono.stop();
	NOTE(_name, " OK (",_chrono.elapsed()/1000,"ms)");
}

void PoolTest::runAll() {
    for(auto& itTest : _mapTests)
        itTest.second->run();
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
	for(auto it = itTest.first; it != itTest.second; it++)
        it->second->run();
}

void PoolTest::getListTests(vector<string>& lTests) {

	for(auto itTest = _mapTests.begin(), end = _mapTests.end(); itTest != end ; itTest = _mapTests.upper_bound(itTest->first)) {
        lTests.push_back(itTest->first);
	}
}

PoolTest& PoolTest::PoolTestInstance () {
	static PoolTest ptest;
	return ptest;
}
