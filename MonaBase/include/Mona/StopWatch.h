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

#include "Mona/Time.h"

namespace Mona {

/// A chrono in microseconds
class Stopwatch : public virtual Object{
public:
	Stopwatch() : _running(false) {}

	void start() { 
		if (_running)
			return;
		_start.update(Time::Now() - elapsed());
		_running = true;
	}

	void stop() { _stop.update(); _running = false; }

	void restart() {
		_start.update();
		_running = true;
	}

	Int64 elapsed() { return _running ? (Time::Now()-_start) : (_stop-_start); }

private:
	Time _start;
	Time _stop;
	bool _running;
};

} // namespace Mona
