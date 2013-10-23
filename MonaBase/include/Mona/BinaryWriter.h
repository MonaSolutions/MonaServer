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
#include "Poco/BinaryWriter.h"
#include "Mona/SocketAddress.h"

namespace Mona {

class BinaryWriter : public Poco::BinaryWriter {
public:
	BinaryWriter(std::ostream& ostr);
	virtual ~BinaryWriter();

	void writeRaw(const Mona::UInt8* value,Mona::UInt32 size);
	void writeRaw(const char* value,Mona::UInt32 size);
	void writeRaw(const std::string& value);
	void write8(Mona::UInt8 value);
	void write16(Mona::UInt16 value);
	void write24(Mona::UInt32 value);
	void write32(Mona::UInt32 value);
	void write64(Mona::UInt64 value);
	void writeString8(const std::string& value);
	void writeString8(const char* value,Mona::UInt8 size);
	void writeString16(const std::string& value);
	void writeString16(const char* value,Mona::UInt16 size);
	void write7BitValue(Mona::UInt32 value);
	void write7BitLongValue(Mona::UInt64 value);
	void writeAddress(const SocketAddress& address,bool publicFlag);

	static BinaryWriter Null;
};

inline void BinaryWriter::writeRaw(const Mona::UInt8* value,Mona::UInt32 size) {
	Poco::BinaryWriter::writeRaw((char*)value,size);
}
inline void BinaryWriter::writeRaw(const char* value,Mona::UInt32 size) {
	Poco::BinaryWriter::writeRaw(value,size);
}
inline void BinaryWriter::writeRaw(const std::string& value) {
	Poco::BinaryWriter::writeRaw(value);
}

inline void BinaryWriter::write8(Mona::UInt8 value) {
	(*this) << value;
}

inline void BinaryWriter::write16(Mona::UInt16 value) {
	(*this) << value;
}

inline void BinaryWriter::write32(Mona::UInt32 value) {
	(*this) << value;
}

inline void BinaryWriter::write64(Mona::UInt64 value) {
	(*this) << value;
}

} // namespace Mona
