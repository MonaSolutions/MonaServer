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


#include "Mona/Event.h"



namespace Mona {

using namespace std;

bool Event::wait(Exception& ex,UInt32 millisec) {
	cv_status result(cv_status::timeout);
	unique_lock<mutex> lock(_mutex);
	try {
		while (!_set) {
			if (millisec>0)
				result = _condition.wait_for(lock, std::chrono::milliseconds(millisec));
			else {
				_condition.wait(lock);
				result = cv_status::no_timeout;
			}	
		}
	} catch (exception& exc) {
		ex.set(Exception::SYSTEM, "Wait event failed, ",exc.what());
	} catch (...) {
		ex.set(Exception::SYSTEM, "Wait event failed, unknown error");
	}
	if (_autoReset && _set)
		_set = false;
	return result == cv_status::no_timeout;
}

void Event::set() {
	unique_lock<mutex> lock(_mutex);
	_set = true;
	_condition.notify_all();
}


void Event::reset() {
	unique_lock<mutex> lock(_mutex);
	_set = false;
}


} // namespace Mona
