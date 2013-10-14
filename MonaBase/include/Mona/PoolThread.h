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
#include "Mona/Startable.h"
#include "Mona/WorkThread.h"
#include "Poco/AtomicCounter.h"
#include "Poco/AutoPtr.h"
#include <list>

namespace Mona {

class PoolThread : private Startable {
public:
	PoolThread();
	virtual ~PoolThread();

	void	clear();
	void	push(Poco::AutoPtr<WorkThread>& pWork);
	int		queue() const;
private:
	void	run();

	Poco::FastMutex							_mutex;
	std::list<Poco::AutoPtr<WorkThread> >	_jobs;
	Poco::AtomicCounter						_queue;
	
	static Mona::UInt32						_Id;
};


} // namespace Mona
