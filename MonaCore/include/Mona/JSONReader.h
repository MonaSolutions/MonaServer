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


class JSONReader : public DataReader, virtual Object {
public:
	JSONReader(PacketReader& packet);

	bool				isValid();

	std::string&		readString(std::string& value);
	double				readNumber();
	bool				readBoolean();
	Date&				readDate(Date& date);
	void				readNull() { packet.next(4); }

	bool				readObject(std::string& type,bool& external);
	bool				readArray(UInt32& size);
	Type				readItem(std::string& name);
	
	Type				followingType();

	void				reset();

private:
	const UInt8*	readBytes(UInt32& size);

	const UInt8*	current();

	UInt32			_pos;
	std::string		_text;
	Date			_date;
	bool			_bool;
	UInt8			_last;

	bool			_validated;
	bool			_isValid;
};


} // namespace Mona
