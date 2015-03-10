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
#include "Mona/Entity.h"
#include <map>

namespace Mona {

struct CompareEntity {
	bool operator()(const UInt8* a,const UInt8* b) const {
		return std::memcmp(a,b,ID_SIZE)<0;
	}
};

template<class EntityType>
class Entities : public virtual Object {
public:

	typedef typename std::map<const UInt8*,EntityType*,CompareEntity>::const_iterator	Iterator;

	Iterator begin() const { return _entities.begin(); }
	Iterator end() const { return _entities.end(); }
	UInt32   count() const { return _entities.size(); }
	bool	 empty() const { return count() == 0; }

	EntityType* operator()(const UInt8* id) const {
		Iterator it = _entities.find(id);
		if(it==_entities.end())
			return NULL;
		return it->second;
	}
	

	Iterator find(const UInt8* id) const {
		return _entities.find(id);
	}
	bool add(EntityType& entity) {
		return _entities.emplace(entity.id,&entity).second;
	}
	Iterator remove(Iterator& it) {
		return _entities.erase(it);
	}
	bool remove(EntityType& entity) {
		return _entities.erase(entity.id)>0;
	}

	EntityType& create(const UInt8* id) {
		auto it = _entities.lower_bound(id);
		if (it != _entities.end() && memcmp(it->first, id, ID_SIZE) == 0)
			return *it->second;
		EntityType* pEntity(new EntityType(id));
		return *_entities.emplace_hint(it, pEntity->id, pEntity)->second;
	}
	void erase(const UInt8* id) {
		auto it(_entities.find(id));
		if (it == _entities.end())
			return;
		delete it->second;
		_entities.erase(it);
	}

private:
	std::map<const UInt8*,EntityType*,CompareEntity> _entities;
};




} // namespace Mona
