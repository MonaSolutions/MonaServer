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

#include "Mona/Trigger.h"


using namespace std;


namespace Mona {

Trigger::Trigger() : _time(1),_cycle(0),_running(false) {
	
}

void Trigger::reset() {
	_timeInit.update();
	_time=1;
	_cycle=0;
}

void Trigger::start() {
	if(_running)
		return;
	reset();
	_running=true;
}

UInt16 Trigger::raise(Exception& ex) {
	if(!_running)
		return 0;
	// Wait at least 1 sec before to begin the repeat cycle, it means that it will be between 1 and 3 sec in truth (freg mangement is set to 2)
	if(_time==1 && !_timeInit.isElapsed(1000))
		return 0;
	++_time;
	if(_time>=_cycle) {
		_time=1;
		++_cycle;
		if (_cycle == 8) {
			ex.set(Exception::PROTOCOL, "Repeat trigger failed");
			return 0;
		}
		return _cycle;
	}
	return 0;
}



} // namespace Mona
