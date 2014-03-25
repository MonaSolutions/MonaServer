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
#include "Mona/Task.h"
#include "Mona/Signal.h"
#include <mutex>


namespace Mona {

class TaskHandler : public virtual Object {
public:
	TaskHandler() : _pTask(NULL), _stop(true) {}
	virtual ~TaskHandler() {stop(); }

	bool waitHandle(Task& task);

protected:
	void start();
	void stop();
	bool running() { return !_stop; }
	void giveHandle(Exception& ex);
private:
	virtual void requestHandle()=0;

	std::recursive_mutex	_mutex;
	std::mutex				_mutexWait;
	Task*					_pTask;
	Signal					_signal;
	volatile bool			_stop;
};



} // namespace Mona
