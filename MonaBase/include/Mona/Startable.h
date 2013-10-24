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
#include "Poco/Thread.h"


namespace Mona {

class Startable;
class StartableProcess : public Poco::Runnable, virtual Object {
public:
	StartableProcess(Startable& startable);
private:
	void run();
	Startable& _startable;
};

class Startable : virtual Object {
	friend class StartableProcess;
public:
	enum WakeUpType {
		WAKEUP=0,
		TIMEOUT,
		STOP
	};

	void				start();
	void				stop();

	WakeUpType			sleep(Exception& ex,UInt32 millisec = 0);
	void				wakeUp() { _wakeUpEvent.set(); }
	void				setPriority(Poco::Thread::Priority priority) { _thread.setPriority(priority); }

	bool				running() const { return !_stop; }
	const std::string&	name() const { return _name; }

protected:
	Startable(const std::string& name);
	virtual ~Startable();

	virtual void	run(Exception& ex) = 0;
	virtual void	prerun(Exception& ex);

private:
	bool					_haveToJoin;
	Poco::Thread			_thread;
	mutable Poco::FastMutex	_mutex;
	mutable Poco::FastMutex	_mutexStop;
	volatile bool			_stop;
	Event					_wakeUpEvent;
	std::string				_name;
	StartableProcess		_process;
};


} // namespace Mona
