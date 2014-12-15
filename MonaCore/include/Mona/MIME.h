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
#include "Mona/DataWriter.h"
#include "Mona/DataReader.h"

namespace Mona {

class MIME : virtual Static {
public:

	enum Type {
		UNKNOWN = 0,
		AMF=1,
		JSON=2,
		XMLRPC=3,
		QUERY
	};

	static Type DataType(DataWriter& writer);
	static Type DataType(DataReader& reader);
	static Type DataType(const char* type);

	static bool CreateDataReader(Type type, PacketReader& packet,const PoolBuffers& poolBuffers,std::unique_ptr<DataReader>& pReader);
	static bool CreateDataWriter(Type type, const PoolBuffers& poolBuffers,std::unique_ptr<DataWriter>& pWriter);
};




} // namespace Mona
