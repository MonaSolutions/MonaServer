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
#include "Mona/Date.h"
#include "Mona/Exceptions.h"
#include "Mona/Logs.h"
#include <thread>

using namespace Mona;
using namespace std;
using namespace std::chrono;

// Test 
ADD_TEST(TimeTest, General) {

	Time t1;
	this_thread::sleep_for(milliseconds(200));
	Time t2;
	Time t3((Int64)t2);
	CHECK(t1 != t2);
	CHECK(!(t1 == t2));
	CHECK(t2 > t1);
	CHECK(t2 >= t1);
	CHECK(!(t1 > t2));
	CHECK(!(t1 >= t2));
	CHECK(t2 == t3);
	CHECK(!(t2 != t3));
	CHECK(t2 >= t3);
	CHECK(t2 <= t3);
	Int64 d = (t2 - t1);
	CHECK(d >= 180 && d <= 300);

	Time epoch(0);
	Int64 tEpoch = epoch;
	CHECK(tEpoch == 0);

	Time now;
	this_thread::sleep_for(milliseconds(201));
	CHECK(now.elapsed() >= 200);
	CHECK(now.isElapsed(200));
	CHECK(!now.isElapsed(2000));

	Time t4;
	Time t4Copy((Int64)t4);
	t4 += 200;
	CHECK(t4 == (t4Copy+200));
	CHECK(t4 == (200+t4Copy));
	t4 -= 200;
	CHECK(t4 == t4Copy);
}



ADD_TEST(TimeTest, DateConversion) {
	Date date,date2;
	for (int i = -100; i <= 2000; ++i) {
		for (int m = 1; m <= 12; ++m) {
			for (int d = 1; d <= 31; ++d) {
				if (d == 31) {
					if (m < 8) {
						if (!(m & 0x01))
							continue;
					} else {
						if (m & 0x01)
							continue;
					}	
				}
				if (m == 2) {
					if (d>29)
						continue;
					if (d == 29 && !Date::IsLeapYear(i))
						continue;
				}
				UInt16 random(Util::Random<UInt16>());
				date.update(i, m, d,random%23,  random%59, random%59, random%1000);
				date2.update(date.time());
				CHECK(date == date2);
			}
		}
		
	}
}

