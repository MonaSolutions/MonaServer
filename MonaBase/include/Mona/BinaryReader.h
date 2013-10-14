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
#include "Poco/BinaryReader.h"
#include "Poco/Net/SocketAddress.h"

namespace Mona {


class BinaryReader : public Poco::BinaryReader {
public:
	BinaryReader(std::istream& istr);
	virtual ~BinaryReader();

	Mona::UInt32	read7BitValue();
	Mona::UInt64	read7BitLongValue();
	Mona::UInt32	read7BitEncoded();
	void			readRaw(Mona::UInt8* value,Mona::UInt32 size);
	void			readRaw(char* value,Mona::UInt32 size);
	void			readRaw(Mona::UInt32 size,std::string& value);
	void			readString8(std::string& value);
	void			readString16(std::string& value);
	Mona::UInt8		read8();
	Mona::UInt16	read16();
	Mona::UInt32	read24();
	Mona::UInt32	read32();
	Mona::UInt64	read64();

	static BinaryReader Null;
};

inline void BinaryReader::readRaw(Mona::UInt8* value,Mona::UInt32 size) {
	Poco::BinaryReader::readRaw((char*)value,size);
}
inline void BinaryReader::readRaw(char* value,Mona::UInt32 size) {
	Poco::BinaryReader::readRaw(value,size);
}
inline void BinaryReader::readRaw(Mona::UInt32 size,std::string& value) {
	Poco::BinaryReader::readRaw(size,value);
}

inline void BinaryReader::readString8(std::string& value) {
	readRaw(read8(),value);
}
inline void BinaryReader::readString16(std::string& value) {
	readRaw(read16(),value);
}

} // namespace Mona
