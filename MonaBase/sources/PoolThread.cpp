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

#include "Mona/PoolThread.h"
#include "Mona/Logs.h"


using namespace std;


namespace Mona {

UInt32	PoolThread::_Id(0);


void PoolThread::run(Exception& exc) {

	for(;;) {

		WakeUpType wakeUpType = sleep(120000); // 2 mn sec of timeout
		
		for (;;) {
			shared_ptr<WorkThread> pWork;
			{
				lock_guard<mutex> lock(_mutex);
				if(_jobs.empty()) {
					if (wakeUpType != WAKEUP) { // STOP or TIMEOUT
						stop(); // to set running=false!
						return;
					}
					break;
				}
				pWork = move(_jobs.front());
				_jobs.pop_front();
			}


#if !defined(_DEBUG)
			try {
#endif
				Exception ex;
				EXCEPTION_TO_LOG(pWork->run(ex),pWork->name);

#if !defined(_DEBUG)
			} catch (exception& ex) {
				ERROR(pWork->name,", ",ex.what());
			} catch (...) {
				ERROR(pWork->name,", unknown error");
			}
#endif

		}
	}
}


} // namespace Mona
