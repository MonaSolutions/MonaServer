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
#include "Mona/DataReader.h"


namespace Mona {


class RawReader : public DataReader, virtual Object {
public:
	RawReader(MemoryReader& reader) : DataReader(reader) {}

	std::string&		readString(std::string& value) { return reader.readRaw(reader.available(), value); }
	double				readNumber() {return 0;}
	bool				readBoolean() {return false;}
	Time&				readTime(Time& time) { return time.update(); }
	void				readNull() {}
	const UInt8*		readBytes(UInt32& size) {return NULL;}

	bool				readObject(std::string& type,bool& external) {return false;}
	bool				readArray(UInt32& size) {return false;}
	Type				readItem(std::string& name) {return END;}
	
	Type				followingType() { return available() ? STRING : END; }

};



} // namespace Mona
