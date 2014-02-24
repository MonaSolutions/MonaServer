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
#include "Mona/StopWatch.h"
#include <thread>

using namespace Mona;
using namespace std;

ADD_TEST(StopWatchTest, TestAll) {
	
	Stopwatch sw;
	sw.start();
	this_thread::sleep_for(chrono::milliseconds(200));
	sw.stop();
	Int64 d = sw.elapsed();
	CHECK(d > 180);
	CHECK(d < 300);

	sw.start();
	this_thread::sleep_for(chrono::milliseconds(100));
	sw.stop();
	d = sw.elapsed();
	CHECK(d > 280);
	CHECK(d < 400);
	
	this_thread::sleep_for(chrono::milliseconds(100));
	sw.stop();
	d = sw.elapsed();
	CHECK(d > 380);
	CHECK(d < 500);
	
	sw.restart();
	sw.start();
	this_thread::sleep_for(chrono::milliseconds(200));
	sw.stop();
	d = sw.elapsed();
	CHECK(d > 180);
	CHECK(d < 300);
}
