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


void Test::run(UInt32 loop) {
	_chrono.restart();
	for (_loop = 0; _loop < loop;++_loop)
		TestFunction();
	_chrono.stop();
	if (loop>1)
		NOTE(_name, " OK (x",_loop," ",_chrono.elapsed(),"ms)")
	else
		NOTE(_name, " OK (",_chrono.elapsed(),"ms)");
}

void PoolTest::runAll(UInt32 loop) {
    for(auto& itTest : _mapTests)
        itTest.second->run(loop);
}

void PoolTest::run(const string& mod,UInt32 loop) {
	auto itTest = _mapTests.equal_range(mod);
	if (itTest.first == itTest.second) 
		itTest = _mapTests.equal_range(mod + "Test");
	if (itTest.first == itTest.second) {
		ERROR("Module ",mod," does not exist.");
		return;
	}

	// Run all tests of the module
	for(auto& it = itTest.first; it != itTest.second; it++)
        it->second->run(loop);
}

void PoolTest::getListTests(vector<string>& lTests) {
	for(auto itTest = _mapTests.begin(), end = _mapTests.end(); itTest != end ; itTest = _mapTests.upper_bound(itTest->first))
        lTests.emplace_back(itTest->first);
}

PoolTest& PoolTest::PoolTestInstance () {
	static PoolTest ptest;
	return ptest;
}
