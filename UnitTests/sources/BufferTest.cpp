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
#include "Mona/Buffer.h"

using namespace Mona;
using namespace std;

static void CheckBuffer(Buffer& buffer,UInt32 capacity=0) {
	CHECK(buffer.capacity()==(capacity == 0 ? 32 : capacity))
	CHECK(buffer.resize(10,false) && buffer.size()==10);
	UInt8* data(buffer.data());
	for (int i = 0; i < 10; ++i)
		*data++ = 'a'+i;
	CHECK(memcmp(buffer.data(), EXPAND("abcdefghij"))==0)
	CHECK(buffer.resize(100,true));
	CHECK(buffer.size()==100);
	CHECK(memcmp(buffer.data(), EXPAND("abcdefghij"))==0)
	CHECK(buffer.capacity()==(capacity == 0 ? 128 : capacity));
	buffer.clip(9);
	CHECK(buffer.size()==91);
	CHECK(*buffer.data()=='j')
	CHECK(buffer.size()>0);
	buffer.clear();
	CHECK(buffer.size()==0);
}

ADD_TEST(BufferTest, Dynamic) {
	Buffer buffer;
	CHECK(buffer.size()==0);
	CheckBuffer(buffer);
}

ADD_TEST(BufferTest, Fix) {
	UInt8 data[100];
	Buffer buffer(data, sizeof(data));
	CHECK(buffer.size()>0);
	CheckBuffer(buffer,sizeof(data));
}
