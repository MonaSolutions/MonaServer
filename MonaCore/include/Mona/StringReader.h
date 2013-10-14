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
#include "Mona/DataReader.h"


namespace Mona {


class StringReader : public DataReader {
public:
	StringReader(MemoryReader& reader) : DataReader(reader) {}
	virtual ~StringReader() {}

	void				readString(std::string& value);
	double				readNumber() {return 0;}
	bool				readBoolean() {return false;}
	Mona::Time		readDate() {return Mona::Time();}
	void				readNull() {}
	const Mona::UInt8*	readBytes(Mona::UInt32& size) {return NULL;}

	bool				readObject(std::string& type,bool& external) {return false;}
	bool				readArray(Mona::UInt32& size) {return false;}
	Type				readItem(std::string& name) {return END;}
	
	Type				followingType();

};

inline StringReader::Type StringReader::followingType() {
	return available() ? STRING : END;
}

inline void StringReader::readString(std::string& value) {
	reader.readRaw(reader.available(),value);
}




} // namespace Mona
