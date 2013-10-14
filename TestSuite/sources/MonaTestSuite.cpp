
#include "TimeTest.h"
#include "gtest/gtest.h"

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	int iRes = RUN_ALL_TESTS();

	system("pause");

	return iRes;
}
