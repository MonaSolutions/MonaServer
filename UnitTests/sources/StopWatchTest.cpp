
#include "StopWatchTest.h"
#include "Mona/StopWatch.h"
#include <thread>

using namespace Mona;
using namespace std;

ADD_TEST(StopWatchTest, TestAll) {
	
	Stopwatch sw;
	sw.start();
	this_thread::sleep_for(chrono::milliseconds(200));
	sw.stop();
	Int64 d = sw.elapsed();
	EXPECT_TRUE(d > 180000);
	EXPECT_TRUE(d < 300000);

	sw.start();
	this_thread::sleep_for(chrono::milliseconds(100));
	sw.stop();
	d = sw.elapsed();
	EXPECT_TRUE(d > 280000);
	EXPECT_TRUE(d < 400000);
	
	this_thread::sleep_for(chrono::milliseconds(100));
	sw.stop();
	d = sw.elapsed();
	EXPECT_TRUE(d > 380000);
	EXPECT_TRUE(d < 500000);
	
	sw.restart();
	sw.start();
	this_thread::sleep_for(chrono::milliseconds(200));
	sw.stop();
	d = sw.elapsed();
	EXPECT_TRUE(d > 180000);
	EXPECT_TRUE(d < 300000);
}
