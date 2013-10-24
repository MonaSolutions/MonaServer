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
using namespace Poco;

namespace Mona {

StartableProcess::StartableProcess(Startable& startable):_startable(startable){
}

void StartableProcess::run() {
	try {
		Exception ex;
		_startable.prerun(ex);
		if (ex)
			CRITIC("Startable thread ",_startable.name(),", ",ex.error());
	} catch(exception& ex) {
		 CRITIC("Startable thread ",_startable.name(),", ",ex.what());
	} catch(...) {
		 CRITIC("Startable thread ",_startable.name(),", error unknown");
	}
}

Startable::Startable(const string& name) : _name(name),_thread(name),_stop(true),_haveToJoin(false),_process(*this) {

}

Startable::~Startable() {
	if(running())
		WARN("Startable::stop should be called by the child class");
	stop();
}

void Startable::start() {
	if(!_stop) // if running
		return;
	ScopedLock<FastMutex> lock(_mutex);
	if(_haveToJoin) {
		_thread.join();
		_haveToJoin=false;
	}
	try {
		ScopedLock<FastMutex> lock(_mutexStop);
		_thread.start(_process);
		_haveToJoin = true;
		_stop=false;
	} catch (Poco::Exception& ex) {
		ERROR("Impossible to start the thread, ",ex.displayText());
	}
}

void Startable::prerun(Exception& ex) {
	run(ex);
	ScopedLock<FastMutex> lock(_mutexStop);
	_stop=true;
}

Startable::WakeUpType Startable::sleep(Exception& ex,UInt32 millisec) {
	if(_stop)
		return STOP;
	 WakeUpType result = WAKEUP;
	 if (!_wakeUpEvent.wait(ex, millisec))
		 result = TIMEOUT;
	if(_stop)
		return STOP;
	return result;
}

void Startable::stop() {
	ScopedLock<FastMutex> lock(_mutex);
	{
		ScopedLock<FastMutex> lock(_mutexStop);
		if(_stop) {
			if(_haveToJoin) {
				_thread.join();
				_haveToJoin=false;
			}
			return;
		}
		_stop=true;
		Thread* pThread = Thread::current();
		if(pThread && pThread->id() == _thread.id())
			return;
	}
	_wakeUpEvent.set();
	// Attendre la fin!
	_thread.join();
	_haveToJoin=false;
}


} // namespace Mona
