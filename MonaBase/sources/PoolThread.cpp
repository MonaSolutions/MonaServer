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
#include "Poco/NumberFormatter.h"

using namespace std;
using namespace Poco;

namespace Mona {

UInt32 PoolThread::_Id = 0;

PoolThread::PoolThread() : Startable("PoolThread"+NumberFormatter::format(++_Id))  {
}

PoolThread::~PoolThread() {
	clear();
}

void PoolThread::clear() {
	stop();
}

void PoolThread::push(AutoPtr<WorkThread>& pWork) {
	++_queue;
	ScopedLock<FastMutex> lock(_mutex);
	_jobs.push_back(pWork);
	start();
	wakeUp();
}

int PoolThread::queue() const {
	return _queue.value();
}

void PoolThread::run() {

	for(;;) {

		WakeUpType wakeUpType = sleep(40000); // 40 sec of timeout
		
		for(;;) {
			WorkThread* pWork;
			{
				ScopedLock<FastMutex> lock(_mutex);
				if(_jobs.empty()) {
					if(wakeUpType!=WAKEUP) { // STOP or TIMEOUT
						if(wakeUpType==TIMEOUT)
							stop();
						return;
					}
					break;
				}
				pWork = _jobs.front();
			}

			pWork->run();
			
			{
				ScopedLock<FastMutex> lock(_mutex);
				_jobs.pop_front();
			}
			--_queue;
		}
	}
}


} // namespace Mona
