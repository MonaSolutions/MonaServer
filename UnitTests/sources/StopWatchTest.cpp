
#include "StopWatchTest.h"
#include "Mona/StopWatch.h"

using namespace Mona;

ADD_TEST(StopWatchTest, TestAll) {
	
	Stopwatch sw;
	sw.start();
	_sleep(200);
	sw.stop();
	Int64 d = sw.elapsed();
	EXPECT_TRUE(d > 180000);
	EXPECT_TRUE(d < 300000);

	sw.start();
	_sleep(100);
	sw.stop();
	d = sw.elapsed();
	EXPECT_TRUE(d > 280000);
	EXPECT_TRUE(d < 400000);
	
	_sleep(100);
	sw.stop();
	d = sw.elapsed();
	EXPECT_TRUE(d > 380000);
	EXPECT_TRUE(d < 500000);
	
	sw.restart();
	sw.start();
	_sleep(200);
	sw.stop();
	d = sw.elapsed();
	EXPECT_TRUE(d > 180000);
	EXPECT_TRUE(d < 300000);
}
