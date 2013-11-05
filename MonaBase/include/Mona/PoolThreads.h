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

#pragma once

#include "Mona/Mona.h"
#include "Mona/PoolThread.h"
#include <vector>

namespace Mona {

class PoolThreads : virtual Object {
public:
	PoolThreads(UInt32 threadsAvailable=0);
	virtual ~PoolThreads();

	void			clear();
	UInt32	threadsAvailable() { return _threads.size(); }

	template<typename WorkThreadType>
	PoolThread* enqueue(Exception& ex,const std::shared_ptr<WorkThreadType>& pWork, PoolThread* pThread = NULL) {
		UInt32 queue = 0;
		if (!pThread) {
			for (PoolThread* pPoolThread : _threads) {
				UInt32 newQueue = pPoolThread->queue();
				if (!pThread || newQueue <= queue) {
					pThread = pPoolThread;
					if ((queue = newQueue) == 0)
						break;
				}
			}
		}
		pThread->push(ex, std::static_pointer_cast<WorkThread>(pWork));
		return pThread;
	}

private:
	std::vector<PoolThread*>	_threads;
};


} // namespace Mona
