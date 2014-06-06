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
#include "Mona/Startable.h"
#include "Mona/WorkThread.h"
#include <memory>
#include <deque>


namespace Mona {

class PoolThread : private Startable, public virtual Object {
public:
	PoolThread() : Startable("PoolThread" + std::to_string(++_Id)) {}
	virtual ~PoolThread() {join();	}

	void	join() { stop(); }

	template<typename WorkThreadType>
	void push(Exception& ex,const std::shared_ptr<WorkThreadType>& pWork) {
		if (!pWork)
			return;
		std::lock_guard<std::mutex> lock(_mutex);
		if (!running() && !start(ex))
			return;
		_jobs.emplace_back(pWork);
		wakeUp();
	}
private:
	void	run(Exception& ex);

	std::mutex								_mutex;
	std::deque<std::shared_ptr<WorkThread>>	_jobs;
	
	static UInt32							_Id;

};


} // namespace Mona
