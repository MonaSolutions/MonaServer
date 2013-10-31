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

ThreadPriority::ThreadPriority(thread& thread) : _thread(thread) {
#if !defined(_WIN32)
	_min = sched_get_priority_min(SCHED_FIFO);
	_max = sched_get_priority_max(SCHED_FIFO);
	// set SCHED_FIFO policy!
	struct sched_param params;
	params.sched_priority = _min + (_max - _min) / 2;
	pthread_setschedparam(threadHandle, SCHED_FIFO, &params);
#endif
}

bool ThreadPriority::set(Priority priority) {
	if (priority == _priority)
		return true;
#if defined(_WIN32)
	if (SetThreadPriority(_thread.native_handle(), priority) == 0)
		return false;
#else
	struct sched_param params;
	if(priority==PRIO_LOWEST)
		params.sched_priority = _min;
	else if(priority==PRIO_LOW)
		params.sched_priority = _min + (_max - _min) / 4;
	else if(priority==PRIO_NORMAL)
		params.sched_priority = _min + (_max - _min) / 2;
	else if (priority == PRIO_HIGH)
		params.sched_priority = _min + (_max - _min) / 4;
	else if (priority == PRIO_HIGHEST)
		params.sched_priority = _max;

	if (pthread_setschedparam(_thread.native_handle(), SCHED_FIFO, &params))
		return false;
#endif
	_priority = priority;
	return true;
}



Startable::Startable(const string& name) : _name(name), _stop(true) {
	
}

Startable::~Startable() {
	stop();
}

void Startable::process() {
	Util::SetThreadName(_thread.get_id(), _name);
	try {
		ThreadPriority priority(_thread);
		Exception ex;
		run(ex, priority);
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
bool Startable::start(Exception& ex) {
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
		_stop = false;
	} catch (exception& exc) {
		ex.set(Exception::THREAD, "Impossible to start the thread of ", _name, ", ", exc.what());
		return false;
	}
	return true;
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
