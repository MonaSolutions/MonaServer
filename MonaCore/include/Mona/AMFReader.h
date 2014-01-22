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
#include "Mona/AMF.h"
#include "Mona/DataReader.h"
#include <deque>

namespace Mona {


class AMFReader : public DataReader, virtual Object {
public:
	AMFReader(PacketReader& reader);

	std::string&		readString(std::string& value);
	double				readNumber();
	bool				readBoolean();
	const UInt8*		readBytes(UInt32& size);
	Time&				readTime(Time& time);
	void				readNull();

	bool			readObject(std::string& type,bool& external);
	bool			readArray(UInt32& size);
	Type			readItem(std::string& name);

	bool			readMap(UInt32& size,bool& weakKeys);
	Type			readKey();
	Type			readValue() { return readKey(); }

	Type			followingType();

	void			startReferencing() { _referencing = true; }
	void			stopReferencing() { _referencing = false; }

	void			reset();

private:
	std::string&					readText(std::string& value);
	UInt8							current() { return *packet.current(); }

	
	std::vector<UInt32>		_stringReferences;
	std::vector<UInt32>		_classDefReferences;
	std::vector<UInt32>		_references;
	std::vector<UInt32>		_amf0References;
	UInt32					_amf0Reset;
	UInt32					_amf3;
	bool					_referencing;

	struct ObjectDef {
		ObjectDef(UInt32 amf3,UInt8 arrayType=0) : amf3(amf3),reset(0),dynamic(false),externalizable(false),count(0),arrayType(arrayType) {}

		std::deque<std::string>		hardProperties;
		UInt32						reset;
		bool						dynamic;
		bool						externalizable;
		UInt32						count;
		UInt8						arrayType;
		const UInt32				amf3;
	};
	std::vector<ObjectDef>	_objectDefs;

};



} // namespace Mona
