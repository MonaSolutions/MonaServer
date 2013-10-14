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
#include "Mona/Listener.h"
#include <map>

namespace Mona {

class Listeners {
public:
	Listeners(std::map<Client*,Listener*>&	listeners) : _listeners(listeners) {}
	virtual ~Listeners() {}

	typedef std::map<Client*,Listener*>::const_iterator Iterator;

	Mona::UInt32 count() const;
	Iterator begin() const;
	Iterator end() const;

private:
	std::map<Client*,Listener*>&	_listeners;
};

inline Mona::UInt32 Listeners::count() const {
	return _listeners.size();
}

inline Listeners::Iterator Listeners::begin() const {
	return _listeners.begin();
}

inline Listeners::Iterator Listeners::end() const {
	return _listeners.end();
}


} // namespace Mona
