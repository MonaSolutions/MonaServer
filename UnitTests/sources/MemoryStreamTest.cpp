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
#include "Mona/MemoryStream.h"


using namespace Mona;

using namespace std;

ADD_TEST(MemoryStreamTest, Input) {

	const char* data = "This is a test";
	MemoryInputStream istr1(data, 14);
	
	int c = istr1.get();
	CHECK(c == 'T');
	c = istr1.get();
	CHECK(c == 'h');
	
	std::string str;
	istr1 >> str;
	CHECK(str == "is");
	
	char buffer[32];
	istr1.read(buffer, sizeof(buffer));
	CHECK(istr1.gcount() == 10);
	buffer[istr1.gcount()] = 0;
	CHECK(std::string(" is a test") == buffer);
	
	const char* data2 = "123";
	MemoryInputStream istr2(data2, 3);
	c = istr2.get();
	CHECK(c == '1');
	CHECK(istr2.good());
	c = istr2.get();
	CHECK(c == '2');
	istr2.unget();
	c = istr2.get();
	CHECK(c == '2');
	CHECK(istr2.good());
	c = istr2.get();
	CHECK(c == '3');
	CHECK(istr2.good());
	c = istr2.get();
	CHECK(c == -1);
	CHECK(istr2.eof());
}


ADD_TEST(MemoryStreamTest, Output) {
	
	char output[64];
	MemoryOutputStream ostr1(output, 64);
	ostr1 << "This is a test " << 42 << std::ends;
	CHECK(ostr1.written() == 18);
	CHECK(std::string("This is a test 42") == output);
	
	char output2[4];
	MemoryOutputStream ostr2(output2, 4);
	ostr2 << "test";
	CHECK(ostr2.good());
	ostr2 << 'x';
	CHECK(ostr2.fail());
}
