
#pragma once

#include "gtest/gtest.h"
#include "Mona/Exceptions.h"

// The fixture for testing class Foo.
class StringTest : public ::testing::Test {
protected:
	// You can remove any or all of the following functions if its body
	// is empty.

	StringTest() {
		// You can do set-up work for each test here.
	}

	virtual ~StringTest() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp() {
		// Code here will be called immediately after the constructor (right
		// before each test).
	}

	virtual void TearDown() {
		// Code here will be called immediately after each test (right
		// before the destructor).
	}
	template<typename T>
	static ::testing::AssertionResult ToNumber(const std::string& value, T expected) { return ToNumber<T>(value.c_str(), expected); }


	template<typename T>
	static ::testing::AssertionResult ToNumber(const char * value, T expected) {

		Mona::Exception ex;
		T result = Mona::String::ToNumber<T>(ex, value);

		if (ex)
			return ::testing::AssertionFailure() << "Exception in ToNumber(" << value << "," << expected << ") : " << ex.error();

		if (result != expected)
			return ::testing::AssertionFailure() << "Invalid Result in ToNumber(" << value << "," << expected << ") : " << result;

		return ::testing::AssertionSuccess();
	}
	
	/// \brief Use FLT_EPSILON by default
	template<>
	static ::testing::AssertionResult ToNumber<float>(const char * value, float expected) { 
		Mona::Exception ex;
		float result = Mona::String::ToNumber<float>(ex, value);

		if (ex)
			return ::testing::AssertionFailure() << "Exception in ToNumber(" << value << "," << expected << ") : " << ex.error();

		if (abs(result -expected) > FLT_EPSILON)
			return ::testing::AssertionFailure() << "Invalid Result in ToNumber(" << value << "," << expected << ") : " << result;

		return ::testing::AssertionSuccess();
	}

	template<>
	static ::testing::AssertionResult ToNumber<double>(const char * value, double expected) { 
		Mona::Exception ex;
		double result = Mona::String::ToNumber<double>(ex, value);

		if (ex)
			return ::testing::AssertionFailure() << "Exception in ToNumber(" << value << "," << expected << ") : " << ex.error();

		if (abs(result -expected) > DBL_EPSILON)
			return ::testing::AssertionFailure() << "Invalid Result in ToNumber(" << value << "," << expected << ") : " << result;

		return ::testing::AssertionSuccess();
	}

	std::string str;
};

