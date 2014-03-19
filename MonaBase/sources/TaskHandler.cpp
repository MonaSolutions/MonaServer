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

#include "Mona/TaskHandler.h"

using namespace std;


namespace Mona {


void TaskHandler::start() {
	lock_guard<recursive_mutex> lock(_mutex);
	_stop=false;
}


void TaskHandler::stop() {
	lock_guard<recursive_mutex> lock(_mutex);
	_stop=true;
	_signal.set();
}

bool TaskHandler::waitHandle(Task& task) {
	lock_guard<mutex> lockWait(_mutexWait);
	{
		lock_guard<recursive_mutex> lock(_mutex);
		if(_stop)
			return false;
		_pTask = &task;
	}
	requestHandle();
	return _signal.wait();
}

void TaskHandler::giveHandle(Exception& ex) {
	lock_guard<recursive_mutex> lock(_mutex);
	if(!_pTask)
		return;
	_pTask->handle(ex);
	_pTask=NULL;
	_signal.set();
}


} // namespace Mona
