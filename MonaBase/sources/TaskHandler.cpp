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

#include "Mona/TaskHandler.h"

using namespace std;
using namespace Poco;

namespace Mona {

TaskHandler::TaskHandler():_pTask(NULL),_stop(true) {
}
TaskHandler::~TaskHandler() {
	stop();
}

void TaskHandler::start() {
	ScopedLock<Mutex> lock(_mutex);
	_stop=false;
}

void TaskHandler::stop() {
	ScopedLock<Mutex> lock(_mutex);
	_stop=true;
	_event.set();
}

void TaskHandler::waitHandle(Task& task) {
	ScopedLock<FastMutex> lockWait(_mutexWait);
	{
		ScopedLock<Mutex> lock(_mutex);
		if(_stop)
			return;
		_pTask = &task;
	}
	requestHandle();
	_event.wait();
}

void TaskHandler::giveHandle() {
	ScopedLock<Mutex> lock(_mutex);
	if(!_pTask)
		return;
	_pTask->handle();
	_pTask=NULL;
	_event.set();
}


} // namespace Mona
