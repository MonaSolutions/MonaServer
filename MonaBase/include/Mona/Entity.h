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

namespace Mona {

#define ID_SIZE 0x20

class Entity : public virtual Object {
public:
	Entity() { std::memset(id, 0, ID_SIZE); }
	Entity(const UInt8* id) { std::memcpy(this->id, id,ID_SIZE);	}
	virtual ~Entity() {}

	bool operator==(const Entity& other) const { return std::memcmp(id, other.id, ID_SIZE) == 0; }
	bool operator==(const UInt8* id) const { return std::memcmp(this->id, id, ID_SIZE) == 0; }
	bool operator!=(const Entity& other) const { return std::memcmp(id, other.id, ID_SIZE) != 0; }
	bool operator!=(const UInt8* id) const { return std::memcmp(this->id, id, ID_SIZE) != 0; }

	UInt8 id[ID_SIZE];
};


} // namespace Mona
