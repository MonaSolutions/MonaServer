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
#include "Mona/Expirable.h"

using namespace std;
using namespace Mona;


class ExpirableObject : public Expirable<ExpirableObject>, public virtual Object {
public:
	ExpirableObject() : Expirable(this) {}
	virtual ~ExpirableObject() {
		expire();
	}

};


ADD_TEST(ExpirableTest, Expire) {
	
	ExpirableObject* pObject = new ExpirableObject();

	Expirable<ExpirableObject> expirable;
	pObject->shareThis(expirable);
	CHECK(pObject->isOwner());
	{
		unique_lock<mutex> lock;
		ExpirableObject* pObject = expirable.safeThis(lock);
		CHECK(pObject)
	}
	delete pObject;

	unique_lock<mutex> lock;
	pObject = expirable.safeThis(lock);
	CHECK(!pObject)
}

