
#include "Test.h"
#include "Mona/Expirable.h"

using namespace std;
using namespace Mona;


class ExpirableObject : public Expirable<ExpirableObject>, virtual Object {
public:
	ExpirableObject() : Expirable(this) {}
	virtual ~ExpirableObject() {
		expire();
	}

};


ADD_TEST(ExpirableObject, Expire) {
	
	ExpirableObject* pObject = new ExpirableObject();

	Expirable<ExpirableObject> expirable;
	Exception ex;
	pObject->shareThis(ex, expirable);
	CHECK(!ex);
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

