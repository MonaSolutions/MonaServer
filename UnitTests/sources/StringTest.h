
#pragma once

#include "Test.h"
#include "Mona/Exceptions.h"
#include "Mona/Logs.h"

// The fixture for testing class Foo.
class StringTest : public Test {
protected:
	// You can remove any or all of the following functions if its body
	// is empty.

	StringTest(const char * testName) : Test(testName) {
		// You can do set-up work for each test here.
	}

	virtual ~StringTest() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	template<typename T>
	static bool ToNumber(const std::string& value, T expected) { return ToNumber<T>(value.c_str(), expected); }


	template<typename T>
	static bool ToNumber(const char * value, T expected) {

		Mona::Exception ex;
		T result = Mona::String::ToNumber<T>(ex, value);

		if (ex) {
			DEBUG("Exception in ToNumber(",value,",",expected,") : ",ex.error());
			return false;
		}

		if (result != expected) {
			DEBUG("Invalid Result in ToNumber(",value,",",expected,") : ",result);
			return false;
		}

		return true;
	}
	
	/// \brief Use FLT_EPSILON by default
	template<>
	static bool ToNumber<float>(const char * value, float expected) { 
		Mona::Exception ex;
		float result = Mona::String::ToNumber<float>(ex, value);

		if (ex) {
			DEBUG("Exception in ToNumber(",value,",",expected,") : ",ex.error());
			return false;
		}

		if (abs(result -expected) > FLT_EPSILON) {
			DEBUG("Invalid Result in ToNumber(",value,",",expected,") : ",result);
			return false;
		}

		return true;
	}

	template<>
	static bool ToNumber<double>(const char * value, double expected) { 
		Mona::Exception ex;
		double result = Mona::String::ToNumber<double>(ex, value);

		if (ex) {
			DEBUG("Exception in ToNumber(",value,",",expected,") : ",ex.error());
			return false;
		}

		if (abs(result -expected) > DBL_EPSILON) {
			DEBUG("Invalid Result in ToNumber(",value,",",expected,") : ",result);
			return false;
		}

		return true;
	}

	std::string str;
};

