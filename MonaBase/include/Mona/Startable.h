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
#include "Mona/Exceptions.h"
#include "Mona/Event.h"
#include <thread>
#if defined(_WIN32)
#include <windows.h>
#endif


namespace Mona {


class ThreadPriority : virtual Object {
	friend class Startable;
public:
	enum Priority {
#if defined(_WIN32)
		LOWEST =	THREAD_PRIORITY_LOWEST,
		LOW =		THREAD_PRIORITY_BELOW_NORMAL,
		NORMAL =	THREAD_PRIORITY_NORMAL,
		HIGH =		THREAD_PRIORITY_ABOVE_NORMAL,
		HIGHEST =	THREAD_PRIORITY_HIGHEST
#else
		LOWEST,
		LOW,
		NORMAL,
		HIGH,
		HIGHEST
#endif
	};


	bool set(Priority priority);

private:
	ThreadPriority(std::thread& thread);

	std::thread&	_thread;
	Priority		_priority;
#if !defined(_WIN32)
	int				_min;
	int				_max;
#endif
};


class Startable : virtual Object {
public:
	enum WakeUpType {
		WAKEUP=0,
		TIMEOUT,
		STOP
	};

	bool				start(Exception& ex);
	void				stop();

	WakeUpType			sleep(UInt32 millisec = 0);
	void				wakeUp() { _wakeUpEvent.set(); }
	

	bool				running() const { return !_stop; }
	const std::string&	name() const { return _name; }

protected:
	Startable(const std::string& name);
	virtual ~Startable();

	virtual void	run(Exception& ex,ThreadPriority& priority) = 0;

private:
	void			process();


	std::thread				_thread;
	std::mutex				_mutex;
	std::mutex				_mutexStop;
	volatile bool			_stop;
	Event					_wakeUpEvent;
	std::string				_name;
};


} // namespace Mona
