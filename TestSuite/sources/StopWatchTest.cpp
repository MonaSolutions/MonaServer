
#include "StopWatchTest.h"
#include "Mona/StopWatch.h"

using namespace Mona;

TEST_F(StopWatchTest, TestStopWatch) {
	
	Stopwatch sw;
	sw.start();
	_sleep(200);
	sw.stop();
	Int64 d = sw.elapsed();
	EXPECT_GE(d, 180000);
	EXPECT_LE(d, 300000);

	sw.start();
	_sleep(100);
	sw.stop();
	d = sw.elapsed();
	EXPECT_GE(d, 280000);
	EXPECT_LE(d, 400000);
	
	_sleep(100);
	sw.stop();
	d = sw.elapsed();
	EXPECT_GE(d, 380000);
	EXPECT_LE(d, 500000);
	
	sw.restart();
	sw.start();
	_sleep(200);
	sw.stop();
	d = sw.elapsed();
	EXPECT_GE(d, 180000);
	EXPECT_LE(d, 300000);
}
