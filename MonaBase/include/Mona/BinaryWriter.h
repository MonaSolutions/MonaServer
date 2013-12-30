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
#include "Mona/SocketAddress.h"
#include <ostream>

namespace Mona {

class BinaryWriter : virtual Object {
public:
	enum ByteOrder {
        BIG_ENDIAN_ORDER,
        LITTLE_ENDIAN_ORDER
	};
    BinaryWriter(std::ostream& ostr, ByteOrder byteOrder = BIG_ENDIAN_ORDER); // BIG_ENDIAN_ORDER==NETWORK_ENDIAN
	virtual ~BinaryWriter();

	void flush() { _ostr.flush(); }

	template <typename ...Args>
	void writeRaw(const UInt8* value, UInt32 size,Args&&... args) {
		_ostr.write((const char*)value, size);
		writeRaw(args ...);
	}
	template <typename ...Args>
	void writeRaw(const char* value, Args&&... args) {
		_ostr.write(value, strlen(value));
		writeRaw(args ...);
	}
	template <typename ...Args>
	void writeRaw(const std::string& value,Args&&... args) {
		_ostr.write(value.c_str(), value.size());
		writeRaw(args ...);
	}

	void write8(UInt8 value) { _ostr.put(value); }
	void write16(UInt16 value);
	void write24(UInt32 value);
	void write32(UInt32 value);
	void write64(UInt64 value);
	void writeString8(const std::string& value) { write8(value.size()); writeRaw(value); }
	void writeString8(const char* value, UInt8 size) { write8(size); _ostr.write(value, size); }
	void writeString16(const std::string& value) { write16(value.size()); writeRaw(value); }
	void writeString16(const char* value, UInt16 size) { write16(size); _ostr.write(value, size); }
	void writeString(const std::string& value) { write7BitEncoded(value.size()); writeRaw(value); }
	void write7BitEncoded(UInt32 value);
	void write7BitValue(UInt32 value);
	void write7BitLongValue(UInt64 value);
	void writeAddress(const SocketAddress& address,bool publicFlag);
	void writeBool(bool value) { _ostr.put(value ? 1 : 0); }
	template<typename NumberType>
	void writeNumber(NumberType value) {
		if (_flipBytes) {
			const char* ptr = (const char*)&value;
			ptr += sizeof(value);
			for (unsigned i = 0; i < sizeof(value); ++i)
				_ostr.write(--ptr, 1);
		} else
			_ostr.write((const char*)&value, sizeof(value));
	}
private:
	void writeRaw() {}

	bool			_flipBytes;
	std::ostream&   _ostr;
};



} // namespace Mona
