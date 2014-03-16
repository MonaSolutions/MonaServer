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
#include "Mona/PacketReader.h"
#include "Mona/DataWriter.h"
#include "Mona/Date.h"

namespace Mona {


class DataReader : virtual NullableObject {
public:
	enum Type {
		NIL=0,
		BOOLEAN,
		NUMBER,
		STRING,
		DATE,
		ARRAY,
		OBJECT,
		BYTES,
		MAP,
		END
	};
	
	virtual Type				followingType()=0;

	virtual std::string&		readString(std::string& value)=0;	
	virtual double				readNumber()=0;
	virtual bool				readBoolean()=0;
	virtual const UInt8*		readBytes(UInt32& size)=0;
	virtual Date&				readDate(Date& date) = 0;
	virtual void				readNull()=0;

	virtual bool				readObject(std::string& type,bool& external)=0;
	virtual bool				readArray(UInt32& size)=0;
	virtual Type				readItem(std::string& name)=0;

	virtual bool				readMap(UInt32& size,bool& weakKeys);
	virtual Type				readKey() { return followingType(); }
	virtual Type				readValue() { return followingType(); }

	virtual bool				available() { return packet.available() > 0; }

	virtual void				reset() { packet.reset(_pos); }

	void						next();
	void						read(DataWriter& writer,UInt32 count=0);

	PacketReader&				packet;

	operator bool() const { return packet; }

protected:
	DataReader(PacketReader& packet);
	DataReader(); // Null

private:
	void						read(Type type,DataWriter& writer);

	UInt32						_pos;
	static PacketReader			_PacketReaderNull;
};


} // namespace Mona
