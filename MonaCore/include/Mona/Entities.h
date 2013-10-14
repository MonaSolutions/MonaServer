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
#include <cstring>
#include <map>

namespace Mona {

template<class EntityType>
class Entities {
public:
	struct Compare {
	   bool operator()(const Mona::UInt8* a,const Mona::UInt8* b) const {
		   return std::memcmp(a,b,ID_SIZE)<0;
	   }
	};

	typedef typename std::map<const Mona::UInt8*,EntityType*,Compare> Map;
	typedef typename Map::const_iterator Iterator;

	Entities(Map& entities) : _entities(entities) {}
	Entities(const Entities& entities) : _entities(entities._entities) {}
	virtual ~Entities(){}


	Iterator		begin() const {
		return _entities.begin();
	}

	Iterator end() const {
		return _entities.end();
	}

	Mona::UInt32 count() const {
		return _entities.size();
	}

	EntityType* operator()(const Mona::UInt8* id) const {
		Iterator it = _entities.find(id);
		if(it==_entities.end())
			return NULL;
		return it->second;
	}
private:
	Map&	_entities;
};




} // namespace Mona
