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
#include "Poco/Thread.h"
#include "Poco/Event.h"

namespace Mona {

class Startable;
class StartableProcess : public Poco::Runnable{
public:
	StartableProcess(Startable& startable);
private:
	void run();
	Startable& _startable;
};

class Startable {
	friend class StartableProcess;
public:
	enum WakeUpType {
		WAKEUP=0,
		TIMEOUT,
		STOP
	};

	void				start();
	void				stop();

	WakeUpType			sleep(Mona::UInt32 timeout=0);
	void				wakeUp();
	void				setPriority(Poco::Thread::Priority priority);

	bool				running() const;		
	const std::string&	name() const;

protected:
	Startable(const std::string& name);
	virtual ~Startable();

	virtual void	run()=0;
	virtual void	prerun();

private:
	bool					_haveToJoin;
	Poco::Thread			_thread;
	mutable Poco::FastMutex	_mutex;
	mutable Poco::FastMutex	_mutexStop;
	volatile bool			_stop;
	Poco::Event				_wakeUpEvent;
	std::string				_name;
	StartableProcess		_process;
};

inline bool Startable::running() const {
	return !_stop;
}


inline void Startable::run() {
	prerun();
}

inline void Startable::wakeUp() {
	_wakeUpEvent.set();
}

inline void Startable::setPriority(Poco::Thread::Priority priority) {
	_thread.setPriority(priority);
}

inline const std::string& Startable::name() const {
	return _name;
}

} // namespace Mona
