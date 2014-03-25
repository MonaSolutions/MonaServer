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

#pragma once

#include "Mona/Mona.h"
#include "Mona/PoolThread.h"
#include "Mona/Util.h"
#include <deque>

namespace Mona {

class PoolThreads : public virtual Object {
public:
	PoolThreads(UInt16 threadsAvailable=0) {
		if (threadsAvailable == 0)
			threadsAvailable = Util::ProcessorCount();
		for(UInt16 i=0;i<threadsAvailable;++i) _threads.emplace_back(new PoolThread());
	}
	virtual ~PoolThreads() {
		for(PoolThread* pThread : _threads) delete pThread;
	}

	void	join() { for(PoolThread* pThread : _threads) pThread->join(); }
	UInt32	threadsAvailable() const { return _threads.size(); }

	template<typename WorkThreadType>
    PoolThread* enqueue(Exception& ex, const std::shared_ptr<WorkThreadType>& pWork, PoolThread* pThread = NULL) {
		if (!pThread) {
			pThread = _threads.front();
			_threads.pop_front();
			_threads.emplace_back(pThread);
		}
		pThread->push<WorkThreadType>(ex, pWork);
		return pThread;
	}

private:
	std::deque<PoolThread*>	_threads;
};


} // namespace Mona
