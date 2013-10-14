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

namespace Mona {

#define ID_SIZE 0x20

class Entity {
public:
	Entity():id(){}
	virtual ~Entity(){}

	bool operator==(const Entity& other) const;
	bool operator==(const Mona::UInt8* id) const;
	bool operator!=(const Entity& other) const;
	bool operator!=(const Mona::UInt8* id) const;

	const Mona::UInt8							id[ID_SIZE];
};


inline bool Entity::operator==(const Entity& other) const {
	return std::memcmp(id,other.id,ID_SIZE)==0;
}
inline bool Entity::operator==(const Mona::UInt8* id) const {
	return std::memcmp(this->id,id,ID_SIZE)==0;
}
inline bool Entity::operator!=(const Entity& other) const {
	return std::memcmp(id,other.id,ID_SIZE)!=0;
}
inline bool Entity::operator!=(const Mona::UInt8* id) const {
	return std::memcmp(this->id,id,ID_SIZE)!=0;
}


} // namespace Mona
