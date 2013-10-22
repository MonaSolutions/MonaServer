
#include "TimeTest.h"
#include "Mona/Time.h"

using namespace Mona;
using namespace std::chrono;

// Test 
TEST_F(TimeTest, TestTimestamp) {

	Time t1;
	_sleep(200);
	Time t2;
	Time t3(t2);
	EXPECT_TRUE(t1 != t2);
	EXPECT_TRUE(!(t1 == t2));
	EXPECT_TRUE(t2 > t1);
	EXPECT_TRUE(t2 >= t1);
	EXPECT_TRUE(!(t1 > t2));
	EXPECT_TRUE(!(t1 >= t2));
	EXPECT_TRUE(t2 == t3);
	EXPECT_TRUE(!(t2 != t3));
	EXPECT_TRUE(t2 >= t3);
	EXPECT_TRUE(t2 <= t3);
	Int64 d = (t2.toInt() - t1.toInt());
	EXPECT_TRUE(d >= 180000 && d <= 300000);

	Time epoch(0);
	Int64 tEpoch = epoch.toInt();
	EXPECT_TRUE(tEpoch == 0);

	Time now;
	_sleep(201);
	EXPECT_TRUE(now.elapsed() >= 200000);
	EXPECT_TRUE(now.isElapsed(200000));
	EXPECT_TRUE(!now.isElapsed(2000000));

	Time t4;
	Time t4Copy(t4);
	t4 += 200;
	EXPECT_TRUE(t4.toInt() == (t4Copy.toInt()+200));
	t4 -= 200;
	EXPECT_TRUE(t4.toInt() == t4Copy.toInt());
}

TEST_F(TimeTest, TestTimeFormat) {

	// GMT time
	struct tm tminit;
	tminit.tm_year = 113;
	tminit.tm_mon = 11;
	tminit.tm_mday = 15;
	tminit.tm_hour = 17;
	tminit.tm_min = 59;
	tminit.tm_sec = 59;
	int milli = 123;
	int micro = 456;
	
	EXPECT_TRUE(Time::isValid(tminit, milli, micro));
	Time time(tminit, milli, micro);

	// Convert Mona Time to GMT time
	struct tm datetm;
	bool bRes = time.toGMT(datetm);

	EXPECT_TRUE(bRes == true);
	if (bRes == true) {
		EXPECT_EQ(tminit.tm_year, datetm.tm_year);
		EXPECT_EQ(tminit.tm_mon, datetm.tm_mon);
		EXPECT_EQ(tminit.tm_mday, datetm.tm_mday);
		EXPECT_EQ(tminit.tm_yday, datetm.tm_yday);
		EXPECT_EQ(tminit.tm_wday, datetm.tm_wday);
		EXPECT_EQ(tminit.tm_hour, datetm.tm_hour);
		EXPECT_EQ(tminit.tm_min, datetm.tm_min);
		EXPECT_EQ(tminit.tm_sec, datetm.tm_sec);
		EXPECT_EQ(milli, time.millisec());
		EXPECT_EQ(micro, time.microsec());
	}

	// Convert Mona Time to Local time
	bRes = time.toLocal(datetm);

	// Convert init time to local time
	time_t utcint = timegm(&tminit);
	tminit = *localtime(&utcint);

	EXPECT_TRUE(bRes == true);
	if (bRes == true) {
		EXPECT_EQ(tminit.tm_year, datetm.tm_year);
		EXPECT_EQ(tminit.tm_mon, datetm.tm_mon);
		EXPECT_EQ(tminit.tm_mday, datetm.tm_mday);
		EXPECT_EQ(tminit.tm_yday, datetm.tm_yday);
		EXPECT_EQ(tminit.tm_wday, datetm.tm_wday);
		EXPECT_EQ(tminit.tm_hour, datetm.tm_hour);
		EXPECT_EQ(tminit.tm_min, datetm.tm_min);
		EXPECT_EQ(tminit.tm_sec, datetm.tm_sec);
		EXPECT_EQ(milli, time.millisec());
		EXPECT_EQ(micro, time.microsec());
	}
}

TEST_F(TimeTest, TestDateTime) {

	Time ts(0); // Unix epoch 1970-01-01 00:00:00 Thursday
	struct tm tmdate;
	bool bRes = ts.toGMT(tmdate);
	EXPECT_TRUE(bRes);

	if (bRes) {
		EXPECT_EQ(tmdate.tm_year, 70);
		EXPECT_EQ(tmdate.tm_mon, 0);
		EXPECT_EQ(tmdate.tm_mday, 1);
		EXPECT_EQ(tmdate.tm_hour, 0);
		EXPECT_EQ(tmdate.tm_min, 0);
		EXPECT_EQ(tmdate.tm_sec, 0);
		EXPECT_EQ(ts.millisec(), 0);
		EXPECT_EQ(tmdate.tm_wday, 4);
		EXPECT_EQ(ts.toInt(), 0);
	}

	ts += 1000000000000000; // 2001-09-09 01:46:40 Sunday
	bRes = ts.toGMT(tmdate);
	EXPECT_TRUE(bRes);

	if (bRes) {
		EXPECT_EQ(tmdate.tm_year, 101);
		EXPECT_EQ(tmdate.tm_mon, 8);
		EXPECT_EQ(tmdate.tm_mday, 9);
		EXPECT_EQ(tmdate.tm_hour, 1);
		EXPECT_EQ(tmdate.tm_min, 46);
		EXPECT_EQ(tmdate.tm_sec, 40);
		EXPECT_EQ(ts.millisec(), 0);
		EXPECT_EQ(tmdate.tm_wday, 0);
		EXPECT_EQ(ts.toInt(), 1000000000000000);
	}
}


TEST_F(TimeTest, TestArithmetics) {

	struct tm tmdate;
	tmdate.tm_year = 105;
	tmdate.tm_mon = 0;
	tmdate.tm_mday = 1;
	tmdate.tm_hour = 0;
	tmdate.tm_min = 15;
	tmdate.tm_sec = 30;

	Time dt1(tmdate);
	tmdate.tm_mday += 1;
	Time dt2(tmdate);

	Int64 microsec = dt2 - dt1;
	EXPECT_EQ(microsec, duration_cast<microseconds>(hours(24)).count());

	Time dt3(dt1 + microsec);
	EXPECT_EQ(dt3, dt2);

	dt3 -= microsec;
	EXPECT_EQ(dt3, dt1);
	dt1 += microsec;
	EXPECT_EQ(dt1, dt2);
}

::testing::AssertionResult IsValid(struct tm& time, int msec, int microsec, bool exp ) {
	if (Time::isValid(time, msec, microsec) == exp)
		return ::testing::AssertionSuccess();
	else
		return ::testing::AssertionFailure() << (time.tm_year + 1900) << "," << (time.tm_mon + 1) << "," << time.tm_mday << "," << time.tm_hour 
				<< "," << time.tm_min << "," << time.tm_sec << "," << msec << "," << microsec << " is not " << ((exp)? "true" : "false");
}

TEST_F(TimeTest, TestIsValid) {

	const struct stTime {

		int  d_year;     // year under test
		int  d_month;    // month under test
		int  d_day;      // day under test
		int  d_hour;		
		int  d_min;			
		int  d_sec;			
		int  d_msec;		
		int  d_microsec;
		bool d_exp;      // expected value
	};
	static stTime data[] = {
		// A valid date is a date between 1/1/1970 and 31/12/3000
		//year   month   day   hour   min   sec   msec   µsec    expected value
		{ 2004, 1, 32, 0, 0, 0, 0, 0, true },
		{ 2004, 2, 30, 0, 0, 0, 0, 0, true },
		{ 2004, 3, 32, 0, 0, 0, 0, 0, true },
		{ 2004, 4, 31, 0, 0, 0, 0, 0, true },
		{ 2004, 5, 32, 0, 0, 0, 0, 0, true },
		{ 2004, 6, 31, 0, 0, 0, 0, 0, true },
		{ 2004, 7, 32, 0, 0, 0, 0, 0, true },
		{ 2004, 8, 32, 0, 0, 0, 0, 0, true },
		{ 2004, 9, 31, 0, 0, 0, 0, 0, true },
		{ 2004, 10, 32, 0, 0, 0, 0, 0, true },
		{ 2004, 11, 31, 0, 0, 0, 0, 0, true },
		{ 2004, 12, 32, 0, 0, 0, 0, 0, true },
		{ 1970, 12, 31, 0, 0, 0, 0, 0, true },
		{ 1971, 1, 1, 0, 0, 0, 0, 0, true },
		{ 2010, 1, 2, 0, 0, 0, 0, 0, true },
		{ 2011, 2, 5, 0, 0, 0, 0, 0, true },
		{ 2012, 3, 10, 0, 0, 0, 0, 0, true },
		{ 2013, 4, 17, 0, 0, 0, 0, 0, true },
		{ 2014, 5, 23, 0, 0, 0, 0, 0, true },
		{ 1970, 2, 29, 0, 0, 0, 0, 0, true },
		{ 2000, 2, 29, 0, 0, 0, 0, 0, true },
		{ 2100, 2, 29, 0, 0, 0, 0, 0, true },
		{ 2001, 3, 18, 16, 30, 15, 900, 890, true },
		{ 2001, 3, 18, 16, 30, 15, 900, 1890, false },
		{ 2001, 3, 18, 16, 30, 15, 900, -1, false },
		{ 2001, 3, 18, 16, 30, 15, 10, 0, true },
		{ 2001, 3, 18, 16, 30, 15, 1000, 0, false },
		{ 2001, 3, 18, 16, 30, 15, -2, 0, false },
		{ 2001, 14, 18, 16, 30, 15, 0, 0, true },
		{ 2001, 14, 18, 25, 90, 78, 0, 0, true },
		{ 1969, 1, 1, 0, 0, 0, 0, 0, false },
		{ 3001, 1, 2, 0, 0, 0, 0, 0, false },
	};

	struct tm time;
	for (stTime tmtmp : data) {

		time.tm_year = tmtmp.d_year - 1900;
		time.tm_mon = tmtmp.d_month - 1;
		time.tm_mday = tmtmp.d_day;
		time.tm_hour = tmtmp.d_hour;
		time.tm_min = tmtmp.d_min;
		time.tm_sec = tmtmp.d_sec;

		EXPECT_TRUE(IsValid(time, tmtmp.d_msec, tmtmp.d_microsec, tmtmp.d_exp));
	}
}
