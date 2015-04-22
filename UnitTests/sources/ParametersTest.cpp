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

static const string Key("key");
static string Value;


ADD_TEST(ParametersTest, MapParameters) {
	MapParameters params;

	bool   bValue;
	double dValue;

	// Set STRING
	params.setString(Key, "1");
	CHECK(params.getString(Key, Value) && Value == "1");
	CHECK(params.getBoolean(Key,bValue) && bValue);
	CHECK(params.getBoolean(Key));
	CHECK(params.getNumber(Key,dValue) && dValue==1);
	CHECK(params.getNumber(Key)==1);
	params.setString(Key, "1",1);
	CHECK(params.getString(Key, Value) && Value == "1");
	

	// Set BOOL
	params.setBoolean(Key, true);
	CHECK(params.getString(Key, Value) && Value == "true");
	CHECK(params.getBoolean(Key,bValue) && bValue);
	CHECK(params.getBoolean(Key));

	params.setBoolean(Key, false);
	CHECK(params.getString(Key, Value) && Value == "false");
	CHECK(params.getBoolean(Key,bValue) && !bValue);
	CHECK(!params.getBoolean<true>(Key));

	// Set Number
	params.setNumber(Key, 1);
	CHECK(params.getString(Key, Value) && Value == "1");
	CHECK(params.getBoolean(Key,bValue) && bValue);
	CHECK(params.getBoolean(Key));
	CHECK(params.getNumber(Key,dValue) && dValue==1);
	CHECK(params.getNumber(Key)==1);

	params.setNumber(Key, 0);
	CHECK(params.getString(Key, Value) && Value == "0");
	CHECK(params.getBoolean(Key,bValue) && !bValue);
	CHECK(!params.getBoolean<true>(Key));
	CHECK(params.getNumber(Key,dValue) && dValue==0);
	CHECK(params.getNumber(Key)==0);

	// Erase
	CHECK(params.count() == 1 && !params.empty());
	params.erase(Key);
	CHECK(params.count() == 0 && params.empty());
	params.setString("hello", EXPAND("mona"));
	CHECK(params.count() == 1 && !params.empty());
	params.clear();
	CHECK(params.count() == 0 && params.empty());
}


