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
#include "Mona/AMF.h"
#include "Mona/DataReader.h"
#include <list>

namespace Mona {


class ObjectDef;
class AMFReader : public DataReader {
public:
	AMFReader(MemoryReader& reader);
	virtual ~AMFReader();

	void				readString(std::string& value);
	double				readNumber();
	bool				readBoolean();
	const Mona::UInt8*	readBytes(Mona::UInt32& size);
	Mona::Time		readDate();
	void				readNull();

	bool			readObject(std::string& type,bool& external);
	bool			readArray(Mona::UInt32& size);
	Type			readItem(std::string& name);

	bool			readMap(Mona::UInt32& size,bool& weakKeys);
	Type			readKey();
	Type			readValue();

	Type			followingType();

	void			startReferencing();
	void			stopReferencing();

	void			reset();

private:
	void							readText(std::string& value);
	Mona::UInt8						current();
	std::list<ObjectDef*>			_objectDefs;
	std::vector<Mona::UInt32>		_stringReferences;
	std::vector<Mona::UInt32>		_classDefReferences;
	std::vector<Mona::UInt32>		_references;
	std::vector<Mona::UInt32>		_amf0References;
	Mona::UInt32					_amf0Reset;
	Mona::UInt32					_amf3;
	bool							_referencing;
};

inline AMFReader::Type AMFReader::readValue() {
	return readKey();
}

inline Mona::UInt8 AMFReader::current() {
	return *reader.current();
}

inline void AMFReader::startReferencing() {
	_referencing=true;
}

inline void	AMFReader::stopReferencing() {
	_referencing=false;
}



} // namespace Mona
