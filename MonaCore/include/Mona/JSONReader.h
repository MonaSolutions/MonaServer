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


class JSONReader : public DataReader, public virtual Object {
public:
	JSONReader(PacketReader& packet,const PoolBuffers& poolBuffers);

	bool				isValid() const { return _isValid; }
	void				reset() { packet.reset(_pos); }

private:
	enum {
		OBJECT =	OTHER,
		ARRAY =		OTHER+1
	};

	bool	readOne(UInt8 type, DataWriter& writer);
	UInt8	followingType();


	const char*		jumpToString(UInt32& size);
	bool			jumpTo(char marker);
	bool			countArrayElement(UInt32& count);
	void			ignoreObjectRest();
	const UInt8*	current();

	UInt32			_size;
	Date			_date;
	double			_number;
	bool			_isValid;
	UInt32			_pos;
	PoolBuffer		_pBuffer;
};


} // namespace Mona
