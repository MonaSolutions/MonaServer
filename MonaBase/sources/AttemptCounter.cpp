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

#include "Mona/AttemptCounter.h"

using namespace std;


namespace Mona {

AttemptCounter::AttemptCounter() {

}

AttemptCounter::~AttemptCounter() {
	// delete attempts
	for(auto it : _attempts)
		delete it.second;
}


void AttemptCounter::clearAttempt(const string& tag) {
	auto it=_attempts.find(tag);
	if(it==_attempts.end())
		return;
	delete it->second;
	_attempts.erase(it);
}

void AttemptCounter::manage() {
	// clean obsolete attempts
	auto it=_attempts.begin();
	while(it!=_attempts.end()) {
		if(it->second->obsolete()) {
			delete it->second;
			_attempts.erase(it++);
		} else
			++it;
	}
}

	




} // namespace Mona
