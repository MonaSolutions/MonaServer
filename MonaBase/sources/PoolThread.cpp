/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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

void PoolThread::push(shared_ptr<WorkThread>& pWork) {
	++_queue;
	lock_guard<mutex> lock(_mutex);
	_jobs.push_back(pWork);
	start();
	wakeUp();
}

void PoolThread::run(Exception& ex) {

	while(!ex) {

		WakeUpType wakeUpType = sleep(ex,40000); // 40 sec of timeout
		
		while(!ex) {
			WorkThread* pWork;
			{
				lock_guard<mutex> lock(_mutex);
				if(_jobs.empty()) {
					if(wakeUpType!=WAKEUP) { // STOP or TIMEOUT
						if(wakeUpType==TIMEOUT)
							stop();
						return;
					}
					break;
				}
				pWork = _jobs.front().get();
			}

			try {
				Exception ex;
				bool success = pWork->run(ex);
				if (ex) {
					if (!success)
						ERROR("Pool thread %s, %s", name().c_str(), ex.error().c_str())
					else
						WARN("Pool thread %s, %s", name().c_str(), ex.error().c_str());
				} else if (!success)
					ERROR("Pool thread %s, unknown error", name().c_str());
			} catch (exception& ex) {
				ERROR("Pool thread %s, %s", name().c_str(), ex.what());
			} catch (...) {
				ERROR("Pool thread %s, unknown error", name().c_str());
			}

			{
				lock_guard<mutex> lock(_mutex);
				_jobs.pop_front();
			}
			--_queue;
		}
	}
}


} // namespace Mona
