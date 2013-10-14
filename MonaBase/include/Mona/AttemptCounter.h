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
#include "Mona/Time.h"
#include <map>

namespace Mona {

class Attempt {
public:
	Attempt() : count(0) {
	}
	~Attempt() {
	}
	Mona::UInt32	count;

	bool obsolete() {
		return _time.isElapsed(120000000); // 2mn
	}
private:
	Mona::Time _time;
};


class AttemptCounter {
public:
	AttemptCounter();
	virtual ~AttemptCounter();
	
	void			manage();

	Mona::UInt32	attempt(const std::string& tag);
	void			clearAttempt(const std::string& tag);

	template<class AttemptType>
	AttemptType&	attempt(const std::string& tag) {
		std::map<std::string,Attempt*>::iterator it = _attempts.lower_bound(tag);
		if(it!=_attempts.end() && it->first==tag) {
			++it->second->count;
			return (AttemptType&)*it->second;
		}
		if(it!=_attempts.begin())
			--it;
		return (AttemptType&)*_attempts.insert(it,std::pair<std::string,Attempt*>(tag,new AttemptType()))->second;
	}

private:
	std::map<std::string,Attempt*>		_attempts;

};

inline Mona::UInt32	AttemptCounter::attempt(const std::string& tag) {
	return (attempt<Attempt>(tag)).count;
}


} // namespace Mona
