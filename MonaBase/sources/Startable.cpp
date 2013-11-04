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


#include "Mona/Startable.h"
#include "Mona/Logs.h"

using namespace std;

namespace Mona {


Startable::Startable(const string& name) : _name(name), _stop(true) {
	
}

Startable::~Startable() {
	stop();
}

void Startable::process() {
	
	try {
		Exception ex;
		run(ex);
		if (ex)
			CRITIC("Startable thread ", _name, ", ", ex.error());
	} catch (exception& ex) {
		CRITIC("Startable thread ", _name, ", ", ex.what());
	} catch (...) {
		CRITIC("Startable thread ", _name, ", error unknown");
	}
	lock_guard<mutex> lock(_mutexStop);
	_stop = true;
}

// Caller is usually the _thread
Startable::WakeUpType Startable::sleep(UInt32 millisec) {
	if (_stop)
		return STOP;
	 WakeUpType result = WAKEUP;
	 if (!_wakeUpEvent.wait(millisec))
		 result = TIMEOUT;
	if(_stop)
		return STOP;
	return result;
}

// caller is usually the thread controller of _thread
bool Startable::start(Exception& ex, Priority priority) {
	if (!_stop)  // if running
		return true;
	lock_guard<mutex> lock(_mutex);
	if (_thread.get_id() == this_thread::get_id()) {
		ex.set(Exception::THREAD,"Startable::start method can't be called from the running thread");
		return false;
	}
	if (_thread.joinable())
		_thread.join();
	try {
		lock_guard<mutex> lock(_mutexStop);
		_thread = thread(&Startable::process, ref(*this)); // start the thread
		_wakeUpEvent.reset();
		initThread(ex, _thread, priority);
		_stop = false;
	} catch (exception& exc) {
		ex.set(Exception::THREAD, "Impossible to start the thread of ", _name, ", ", exc.what());
		return false;
	}
	return true;
}


void Startable::initThread(Exception& ex,thread& thread,Priority priority) {
	Util::SetThreadName(thread.get_id(), _name);

#if !defined(_WIN32)
	static int Min = sched_get_priority_min(SCHED_FIFO);
	static int Max = sched_get_priority_max(SCHED_FIFO);
	static int Priorities[] = {Min,Min + (Max - Min) / 4,Min + (Max - Min) / 2,Min + (Max - Min) / 4,Max};
#endif

#if defined(_WIN32)
	if (priority == PRIORITY_NORMAL || SetThreadPriority(_thread.native_handle(), priority) != 0)
		return;
#else
	struct sched_param params;
	params.sched_priority = Priorities[priority];
	if (!pthread_setschedparam(_thread.native_handle(), SCHED_FIFO, &params))
		return;
#endif
	ex.set(Exception::THREAD, "Impossible to change the thread ", _name, " priority to ", priority);
}


// caller is usually the thread controller of _thread
void Startable::stop() {
	lock_guard<mutex> lock(_mutex);
	{
		lock_guard<mutex> lock(_mutexStop);
		if (_stop) {
			if (_thread.get_id() != this_thread::get_id() && _thread.joinable())
				_thread.join();
			return;
		}
		_stop = true;
	}
	_wakeUpEvent.set();
	if (_thread.get_id() != this_thread::get_id() && _thread.joinable())
		_thread.join();
}


} // namespace Mona
