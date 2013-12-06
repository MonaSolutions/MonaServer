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
#include "Mona/Logs.h"
#if !defined(_WIN32)
#include <sys/prctl.h> // for thread name
#endif


using namespace std;

namespace Mona {


Startable::Startable(const string& name) : _name(name), _stop(true) {
	
}

Startable::~Startable() {
	stop();
}

void Startable::setDebugThreadName() {
#if defined(_DEBUG)
#if defined(_WIN32)
	typedef struct tagTHREADNAME_INFO {
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	} THREADNAME_INFO;

	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = _name.c_str();
	info.dwThreadID = GetCurrentThreadId();
	info.dwFlags = 0;

	__try {
		RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
	}

#else
	prctl(PR_SET_NAME, name.c_str(), 0, 0, 0);
#endif
#endif
}

void Startable::process() {
	setDebugThreadName();
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
	lock_guard<mutex> lock(_mutex);
	if (!_stop)  // if running
		return true;
	if (_thread.get_id() == this_thread::get_id()) {
		ex.set(Exception::THREAD,"Startable::start method can't be called from the running thread");
		return false;
	}
	lock_guard<mutex> lockStop(_mutexStop);
	if (_thread.joinable())
		_thread.join();
	try {
		_wakeUpEvent.reset();
		_stop = false;
		_thread = thread(&Startable::process, ref(*this)); // start the thread
		initThread(ex, _thread, priority);
	} catch (exception& exc) {
		ex.set(Exception::THREAD, "Impossible to start the thread of ", _name, ", ", exc.what());
		_stop = true;
		return false;
	}
	return true;
}


void Startable::initThread(Exception& ex,thread& thread,Priority priority) {
	Util::SetThreadName(thread.get_id(), _name);
#if defined(_WIN32)
	if (priority == PRIORITY_NORMAL || SetThreadPriority(_thread.native_handle(), priority) != 0)
		return;
#else
	static int Min = sched_get_priority_min(SCHED_FIFO);
	static int Max = sched_get_priority_max(SCHED_FIFO);
	static int Priorities[] = {Min,Min + (Max - Min) / 4,Min + (Max - Min) / 2,Min + (Max - Min) / 4,Max};

	struct sched_param params;
	params.sched_priority = Priorities[priority];
	if (!pthread_setschedparam(_thread.native_handle(), SCHED_FIFO, &params))
		return;
#endif
	ex.set(Exception::THREAD, "Impossible to change the thread ", _name, " priority to ", priority);
}


// caller is usually the thread controller of _thread
void Startable::stop() {
	if (_thread.get_id() == this_thread::get_id()) {
		// case where it's the thread which calls stop!
		lock_guard<mutex> lock(_mutexStop);
		if (_stop)
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
	_wakeUpEvent.set();
	if (_thread.joinable())
		_thread.join();
}


} // namespace Mona
