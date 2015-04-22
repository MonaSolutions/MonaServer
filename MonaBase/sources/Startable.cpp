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


#include "Mona/Startable.h"
#if defined(linux)
#include <sys/prctl.h> // for thread name
#elif defined(_WIN32)
#include <windows.h>
#endif
#include "Mona/Logs.h"


using namespace std;

namespace Mona {


Startable::Startable(const string& name) : _priority(PRIORITY_NORMAL),_name(name), _stop(true) {
	
}

Startable::~Startable() {
	stop();
}

void Startable::process() {
	Util::SetCurrentThreadName(_name.c_str());

	// set priority
#if defined(_WIN32)
	static int Priorities[] = { THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_HIGHEST };
	if (_priority != PRIORITY_NORMAL && SetThreadPriority(GetCurrentThread(), Priorities[_priority]) == 0)
		WARN("Impossible to change the thread ", _name, " priority to ", Priorities[_priority]);
#else
	static int Min = sched_get_priority_min(SCHED_OTHER);
	if(Min==-1) {
		WARN("Impossible to compute minimum thread ", _name, " priority, ",strerror(errno));
	} else {
		static int Max = sched_get_priority_max(SCHED_OTHER);
		if(Max==-1) {
			WARN("Impossible to compute maximum thread ", _name, " priority, ",strerror(errno));
		} else {
			static int Priorities[] = {Min,Min + (Max - Min) / 4,Min + (Max - Min) / 2,Min + (Max - Min) / 4,Max};

			struct sched_param params;
			params.sched_priority = Priorities[_priority];
			int result;
			if ((result=pthread_setschedparam(pthread_self(), SCHED_OTHER , &params)))
				WARN("Impossible to change the thread ", _name, " priority to ", Priorities[_priority]," ",strerror(result));
		}
	}
#endif

#if !defined(_DEBUG)
	try {
#endif
		Exception ex;
		run(ex);
		if (ex)
			CRITIC("Startable thread ", _name, ", ", ex.error());
#if !defined(_DEBUG)
	} catch (exception& ex) {
		CRITIC("Startable thread ", _name, ", ", ex.what());
	} catch (...) {
		CRITIC("Startable thread ", _name, ", error unknown");
	}
#endif
	lock_guard<mutex> lock(_mutexStop);
	_stop = true;
}

// Caller is usually the _thread
Startable::WakeUpType Startable::sleep(UInt32 millisec) {
	if (_stop)
		return STOP;
	 WakeUpType result = WAKEUP;
	 if (!_wakeUpSignal.wait(millisec))
		 result = TIMEOUT;
	if(_stop)
		return STOP;
	return result;
}

// caller is usually the thread controller of _thread
bool Startable::start(Exception& ex, Priority priority) {
	lock_guard<mutex> lock(_mutex);
	if (!_stop)  // if running
		return true;
	lock_guard<mutex> lockStop(_mutexStop);
	if (_thread.joinable())
		_thread.join();
	try {
		_wakeUpSignal.reset();
		_stop = false;
		_priority = priority;
		_thread = thread(&Startable::process, this); // start the thread
	} catch (exception& exc) {
		ex.set(Exception::THREAD, "Impossible to start the thread of ", _name, ", ", exc.what());
		_stop = true;
		return false;
	}
	return true;
}


// caller is usually the thread controller of _thread
void Startable::stop() {
	if (_thread.get_id() == this_thread::get_id()) {
		// case where it's the thread which calls stop!
		lock_guard<mutex> lock(_mutexStop);
		_stop = true;
		return;
	}
	lock_guard<mutex> lock(_mutex);
	{
		lock_guard<mutex> lock(_mutexStop);
		if (_stop) {
			if (_thread.joinable())
				_thread.join();
			return;
		}
		_stop = true;
	}
	_wakeUpSignal.set();
	if (_thread.joinable())
		_thread.join();
}


} // namespace Mona
