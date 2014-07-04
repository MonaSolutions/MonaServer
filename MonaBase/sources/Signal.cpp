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

#include "Mona/Signal.h"
#include "Mona/Exceptions.h"


namespace Mona {

using namespace std;

bool Signal::wait(UInt32 millisec) {
	cv_status result(cv_status::no_timeout);
	unique_lock<mutex> lock(_mutex);
#if !defined(_DEBUG)
	try {
#endif
		while (!_set && result == cv_status::no_timeout) {
			if (millisec > 0)
				result = _condition.wait_for(lock, std::chrono::milliseconds(millisec));
			else
				_condition.wait(lock);
		}
#if !defined(_DEBUG)
	} catch (exception& exc) {
		FATAL_ERROR("Wait signal failed, ", exc.what());
	} catch (...) {
		FATAL_ERROR("Wait signal failed, unknown error");
	}
#endif
	if (_autoReset && _set)
		_set = false;
	return result == cv_status::no_timeout;
}

void Signal::set() {
	unique_lock<mutex> lock(_mutex);
	_set = true;
	_condition.notify_all();
}


void Signal::reset() {
	unique_lock<mutex> lock(_mutex);
	_set = false;
}


} // namespace Mona
