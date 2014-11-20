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
#include "Mona/Exceptions.h"
#include "Mona/Signal.h"
#include <thread>


namespace Mona {



class Startable : public virtual Object {
public:
	enum Priority {
		PRIORITY_LOWEST=0,
		PRIORITY_LOW,
		PRIORITY_NORMAL,
		PRIORITY_HIGH,
		PRIORITY_HIGHEST
	};

	enum WakeUpType {
		WAKEUP=0,
		TIMEOUT,
		STOP
	};

	bool				start(Exception& ex, Priority priority = PRIORITY_NORMAL);
	void				stop();

	WakeUpType			sleep(UInt32 millisec = 0);
	void				wakeUp() { _wakeUpSignal.set(); }
	

	bool				running() const { return !_stop; }
	const std::string&	name() const { return _name; }

protected:
	Startable(const std::string& name);
	virtual ~Startable();

	virtual void	run(Exception& ex) = 0;

private:
	void			process();
	

	std::thread				_thread;
	std::mutex				_mutex;
	std::mutex				_mutexStop;
	volatile bool			_stop;
	Signal					_wakeUpSignal;
	std::string				_name;
	Priority				_priority;
};


} // namespace Mona
