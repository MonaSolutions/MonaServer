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
#include <condition_variable>

namespace Mona {


class Signal : public virtual Object {
public:
	Signal(bool autoReset=true) : _autoReset(autoReset), _set(false) {}

	void set();

	// return true if the event has been set
	bool wait(UInt32 millisec = 0);

	void reset();
	
private:
	bool					_autoReset;
	bool					_set;
	std::condition_variable _condition;
	std::mutex				_mutex;
};


} // namespace Mona
