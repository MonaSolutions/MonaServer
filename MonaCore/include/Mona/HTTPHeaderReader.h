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
#include "Mona/Time.h"


namespace Mona {


class HTTPHeaderReader : public DataReader, public virtual Object {
public:
	HTTPHeaderReader(std::vector<const char*>& headers) : _header(headers.begin()), _headers(headers) {}

	void				reset() { _header = _headers.begin(); }

private:
	bool	readOne(UInt8 type, DataWriter& writer);
	UInt8	followingType() { return _header == _headers.end() ? END : OTHER; }

	std::vector<const char*>&				 _headers;
	std::vector<const char*>::const_iterator _header;
};



} // namespace Mona
