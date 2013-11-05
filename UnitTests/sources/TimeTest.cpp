
#include "Test.h"
#include "Mona/Time.h"
#include "Mona/Exceptions.h"
#include "Mona/Logs.h"
#include <thread>

using namespace Mona;
using namespace std;
using namespace std::chrono;
using namespace std;

// Test 
ADD_TEST(TimeTest, TestTimestamp) {

	Time t1;
	this_thread::sleep_for(milliseconds(200));
	Time t2;
	Time t3((Int64)t2);
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
	Int64 d = (t2 - t1);
	EXPECT_TRUE(d >= 180000 && d <= 300000);

	Time epoch(0);
	Int64 tEpoch = epoch;
	EXPECT_TRUE(tEpoch == 0);

	Time now;
	this_thread::sleep_for(milliseconds(201));
	EXPECT_TRUE(now.elapsed() >= 200000);
	EXPECT_TRUE(now.isElapsed(200000));
	EXPECT_TRUE(!now.isElapsed(2000000));

	Time t4;
	Time t4Copy((Int64)t4);
	t4 += 200;
	EXPECT_TRUE(t4 == (t4Copy+200));
	EXPECT_TRUE(t4 == (200+t4Copy));
	t4 -= 200;
	EXPECT_TRUE(t4 == t4Copy);
}

ADD_TEST(TimeTest, TestTimeFormat) {

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
	
	EXPECT_TRUE(Time::IsValid(tminit, milli, micro));
	Time time(tminit, milli, micro);

	// Convert Mona Time to GMT time
	struct tm datetm;
	time.toGMT(datetm);

	EXPECT_TRUE(tminit.tm_year == datetm.tm_year);
	EXPECT_TRUE(tminit.tm_mon == datetm.tm_mon);
	EXPECT_TRUE(tminit.tm_mday == datetm.tm_mday);
	EXPECT_TRUE(tminit.tm_yday == datetm.tm_yday);
	EXPECT_TRUE(tminit.tm_wday == datetm.tm_wday);
	EXPECT_TRUE(tminit.tm_hour == datetm.tm_hour);
	EXPECT_TRUE(tminit.tm_min == datetm.tm_min);
	EXPECT_TRUE(tminit.tm_sec == datetm.tm_sec);
	EXPECT_TRUE(milli == time.millisec());
	EXPECT_TRUE(micro == time.microsec());

	// Convert Mona Time to Local time
	time.toLocal(datetm);

	// Convert init time to local time
	time_t utcint = timegm(&tminit);
	struct tm tmlocal;
	LOCALTIME(utcint, tmlocal);

	EXPECT_TRUE(tmlocal.tm_year == datetm.tm_year);
	EXPECT_TRUE(tmlocal.tm_mon == datetm.tm_mon);
	EXPECT_TRUE(tmlocal.tm_mday == datetm.tm_mday);
	EXPECT_TRUE(tmlocal.tm_yday == datetm.tm_yday);
	EXPECT_TRUE(tmlocal.tm_wday == datetm.tm_wday);
	EXPECT_TRUE(tmlocal.tm_hour == datetm.tm_hour);
	EXPECT_TRUE(tmlocal.tm_min == datetm.tm_min);
	EXPECT_TRUE(tmlocal.tm_sec == datetm.tm_sec);
	EXPECT_TRUE(milli == time.millisec());
	EXPECT_TRUE(micro == time.microsec());
}

ADD_TEST(TimeTest, TestDateTime) {

	Time ts(0); // Unix epoch 1970-01-01 00:00:00 Thursday
	struct tm tmdate;
	ts.toGMT(tmdate);
	
	EXPECT_TRUE(tmdate.tm_year == 70);
	EXPECT_TRUE(tmdate.tm_mon == 0);
	EXPECT_TRUE(tmdate.tm_mday == 1);
	EXPECT_TRUE(tmdate.tm_hour == 0);
	EXPECT_TRUE(tmdate.tm_min == 0);
	EXPECT_TRUE(tmdate.tm_sec == 0);
	EXPECT_TRUE(ts.millisec() == 0);
	EXPECT_TRUE(tmdate.tm_wday == 4);
	EXPECT_TRUE(ts == 0);

	ts += 1000000000000000; // 2001-09-09 01:46:40 Sunday
	ts.toGMT(tmdate);

	EXPECT_TRUE(tmdate.tm_year == 101);
	EXPECT_TRUE(tmdate.tm_mon == 8);
	EXPECT_TRUE(tmdate.tm_mday == 9);
	EXPECT_TRUE(tmdate.tm_hour == 1);
	EXPECT_TRUE(tmdate.tm_min == 46);
	EXPECT_TRUE(tmdate.tm_sec == 40);
	EXPECT_TRUE(ts.millisec() == 0);
	EXPECT_TRUE(tmdate.tm_wday == 0);
	EXPECT_TRUE(ts == 1000000000000000);
}


ADD_TEST(TimeTest, TestArithmetics) {

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
	EXPECT_TRUE(microsec == duration_cast<microseconds>(hours(24)).count());

	Time dt3(dt1 + microsec);
	EXPECT_TRUE(dt3 == dt2);

	dt3 -= microsec;
	EXPECT_TRUE(dt3 == dt1);
	dt1 += microsec;
	EXPECT_TRUE(dt1 == dt2);
}

bool IsValid(struct tm& time, int msec, int microsec, bool exp ) {
	if (Time::IsValid(time, msec, microsec) == exp)
		return true;
	else {
		ERROR((time.tm_year + 1900),",",(time.tm_mon + 1),",",time.tm_mday,",",time.tm_hour 
				,",",time.tm_min,",",time.tm_sec,",",msec,",",microsec," is not ",((exp)? "true" : "false"));
		return false;
	}
}

ADD_TEST(TimeTest, TestIsValid) {

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
