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
#include "Mona/Time.h"


namespace Mona {


class HTTPPacketReader : public DataReader {
public:
	HTTPPacketReader(MemoryReader& reader);
	virtual ~HTTPPacketReader();

	void				readString(std::string& value);
	double				readNumber();
	bool				readBoolean();
	Time&				readTime(Time& time);
	void				readNull();

	bool				readObject(std::string& type,bool& external);
	Type				readItem(std::string& name);
	
	Type				followingType();

	void				reset();

private:
	bool				readArray(Poco::UInt32& size) {return false;}
	const Poco::UInt8*	readBytes(Poco::UInt32& size);
	
	void				readLine();

	Type				_type;
	double				_number;
	std::string			_value;
	std::string			_name;
	Time			_date;
};

inline bool HTTPPacketReader::readObject(std::string& type,bool& external) {
	return true;
}


} // namespace Mona
