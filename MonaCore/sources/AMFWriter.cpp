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

#include "Mona/AMFWriter.h"
#include "Mona/Logs.h"
#include <math.h>

using namespace std;


namespace Mona {

AMFWriter   AMFWriter::Null;

AMFWriter::AMFWriter(const PoolBuffers& poolBuffers) : _amf0References(0),_amf3(false),amf0(false),DataWriter(poolBuffers) {

}

void AMFWriter::clear(UInt32 size) {
	_amf3 = false;
	_levels.clear();
	_references.clear();
	_amf0References = 0;
	_stringReferences.clear();
	DataWriter::clear(size);
}

bool AMFWriter::repeat(UInt64 reference) {
	if (reference & 0x01) {
		// AMF3
		reference >>= 1;
		if(--reference>=_references.size()) {
			ERROR("No AMF3 reference to repeat");
			return false;
		}
		if (!_amf3)
			packet.write8(AMF_AVMPLUS_OBJECT);
		packet.write8(_references[(vector<UInt8>::size_type&)reference]);
		packet.write7BitValue((UInt32)reference<<1);
		return true;
	}

	// AMF0
	reference >>= 1;
	if(--reference>=_amf0References) {
		ERROR("No AMF0 reference to repeat");
		return false;
	}
	packet.write8(AMF_REFERENCE);
	packet.write16((UInt16)reference);
	return true;
}

void AMFWriter::writeString(const char* value, UInt32 size) {
	if(!_amf3) {
		if(amf0) {
			if(size>65535) {
				packet.write8(AMF_LONG_STRING);
				packet.write32(size);
			} else {
				packet.write8(AMF_STRING);
				packet.write16(size);
			}
			packet.write(value,size);
			return;
		}
		packet.write8(AMF_AVMPLUS_OBJECT);
	}
	packet.write8(AMF3_STRING); // marker
	writeText(value,size);
	return;
}

void AMFWriter::writePropertyName(const char* name) {
	UInt32 size(strlen(name));
	// no marker, no string_long, no empty value
	if(!_amf3) {
		packet.write16(size).write(name,size);
		return;
	}
	writeText(name,size);
}

void AMFWriter::writeText(const char* value,UInt32 size) {
	if(size>0) {
		auto it = _stringReferences.lower_bound(value);
		if(it!=_stringReferences.end() && it->first==value) {
			packet.write7BitValue(it->second<<1);
			return ;
		}
		_stringReferences.emplace_hint(it, piecewise_construct, forward_as_tuple(value,size), forward_as_tuple(_stringReferences.size()));
	}
	packet.write7BitValue((size<<1) | 0x01).write(value,size);
}

void AMFWriter::writeNull() {
	packet.write8(_amf3 ? AMF3_NULL : AMF_NULL); // marker
}

void AMFWriter::writeBoolean(bool value){
	if(!_amf3)
		packet.write8(AMF_BOOLEAN).writeBool(value);
	else
		packet.write8(value ? AMF3_TRUE : AMF3_FALSE);
}

UInt64 AMFWriter::writeDate(const Date& date){
	if(!_amf3) {
		if(amf0) {
			packet.write8(AMF_DATE);
			packet.writeNumber<double>((double)date);
			packet.write16(0); // Timezone, useless in AMF0 format (always equals 0)
			return 0;
		}
		packet.write8(AMF_AVMPLUS_OBJECT);
	}
	packet.write8(AMF3_DATE);
	packet.write8(0x01);
	packet.writeNumber<double>((double)date);
	_references.emplace_back(AMF3_DATE);
	return (_references.size()<<1) | 1;
}

void AMFWriter::writeNumber(double value){
	if(!amf0 && round(value) == value && abs(value)<=AMF_MAX_INTEGER) {
		// writeInteger
		if(!_amf3)
			packet.write8(AMF_AVMPLUS_OBJECT);
		packet.write8(AMF3_INTEGER); // marker
		if(value<0)
			value+=(1<<29);
		packet.write7BitValue((UInt32)value);
		return;
	}
	packet.write8(_amf3 ? AMF3_NUMBER : AMF_NUMBER); // marker
	packet.writeNumber<double>(value);
}

UInt64 AMFWriter::writeBytes(const UInt8* data,UInt32 size) {
	if (!_amf3) {
		if (amf0)
			WARN("Impossible to write a byte array in AMF0, switch to AMF3");
		packet.write8(AMF_AVMPLUS_OBJECT); // switch in AMF3 format
	}
	packet.write8(AMF3_BYTEARRAY); // bytearray in AMF3 format!
	packet.write7BitValue((size << 1) | 1);
	packet.write(data,size);
	_references.emplace_back(AMF3_BYTEARRAY);
	return (_references.size()<<1) | 0x01;
}

UInt64 AMFWriter::beginObject(const char* type) {
	_levels.push_back(_amf3);
	if(!_amf3) {
		if(amf0) {	
			if (!type)
				packet.write8(AMF_BEGIN_OBJECT);
			else {
				packet.write8(AMF_BEGIN_TYPED_OBJECT);
				UInt16 size((UInt16)strlen(type));
				packet.write16(size).write(type,size);
			}
			return (++_amf0References)<<1;
		}
		packet.write8(AMF_AVMPLUS_OBJECT);
		_amf3=true;
	}
	packet.write8(AMF3_OBJECT);

	/* Can be ignored finally!
	if(externalizable) {
		// What follows is the value of the “inner” object
	} else if(hardProperties>0 && !pClassDef) {
		// The remaining integer-data represents the number of class members that exist.
		// If there is a class-def reference there are no property names and the number of values is equal to the number of properties in the class-def
		flags |= (hardProperties<<4);
	}*/

	// ClassDef always inline (because never hard properties, all is dynamic)
	// Always dynamic (but can't be externalizable AND dynamic!)
	packet.write7BitValue(11); // 00001011 => inner object + classdef inline + dynamic

	writePropertyName(type ? type : "");

	_references.emplace_back(AMF3_OBJECT);
	return (_references.size() << 1) | 1;
}

UInt64 AMFWriter::beginArray(UInt32 size) {
	_levels.push_back(_amf3);
	if(!_amf3) {
		if(amf0) {
			packet.write8(AMF_STRICT_ARRAY);
			packet.write32(size);
			return (++_amf0References)<<1;
		}
		packet.write8(AMF_AVMPLUS_OBJECT);
		_amf3=true;
	}

	packet.write8(AMF3_ARRAY);
	packet.write7BitValue((size << 1) | 1);
	packet.write8(01); // end marker, no properties (pure array)
	_references.emplace_back(AMF3_ARRAY);
	return (_references.size()<<1) | 1;
}

UInt64 AMFWriter::beginObjectArray(UInt32 size) {
	_levels.push_back(_amf3); // endObject
	if(!_amf3) {
		if (amf0)
			WARN("Mixed object in AMF0 are not supported, switch to AMF3");
		packet.write8(AMF_AVMPLUS_OBJECT);
		_amf3=true;
	}
	_levels.push_back(_amf3); // endArray
	packet.write8(AMF3_ARRAY);
	packet.write7BitValue((size << 1) | 1);
	_references.emplace_back(AMF3_ARRAY);
	return (_references.size()<<1) | 1;
}

UInt64 AMFWriter::beginMap(Exception& ex, UInt32 size,bool weakKeys) {
	_levels.push_back(_amf3);
	if(!_amf3) {
		if (amf0)
			WARN("Impossible to write a map in AMF0, switch to AMF3");
		packet.write8(AMF_AVMPLUS_OBJECT);
		_amf3=true;
	}
	packet.write8(AMF3_DICTIONARY);
	packet.write7BitValue((size << 1) | 1);
	packet.write8(weakKeys ? 0x01 : 0x00);
	_references.emplace_back(AMF3_DICTIONARY);
	return (_references.size()<<1) | 1;
}

void AMFWriter::endComplex(bool isObject) {
	if(_levels.empty()) {
		ERROR("endComplex called without beginComplex calling");
		return;
	}

	if(isObject) {
		if(!_amf3) {
			packet.write16(0); 
			packet.write8(AMF_END_OBJECT);
		} else
			packet.write8(01); // end marker
	}

	_amf3 = _levels.back();
	_levels.pop_back();
}


} // namespace Mona
