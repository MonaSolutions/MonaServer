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
#include "Mona/MapParameters.h"

using namespace std;
using namespace Mona;

static MapParameters Params;
static string Buffer;


ADD_TEST(MapParametersTest, Creation) {
	MapParameters params;
}

ADD_TEST(MapParametersTest, Set) {
	static UInt32 I(0);
	Params.setString(String::Format(Buffer,"key",++I), "value");
	CHECK(Params.count() == I);
}

ADD_TEST(MapParametersTest, Get) {
	static string Value;
	CHECK(Params.getString(Buffer, Value) && Value=="value");
}
