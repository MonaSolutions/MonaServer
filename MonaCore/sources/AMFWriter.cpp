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

AMFWriter::AMFWriter(const PoolBuffers& poolBuffers) : DataWriter(poolBuffers),_amf3(false),amf0Preference(false) {

}

void AMFWriter::clear() {
	_amf3=false;
	amf0Preference=false;
	_lastObjectReferences.clear();
	_references.clear();
	_stringReferences.clear();
	DataWriter::clear();
}

bool AMFWriter::repeat(UInt32 reference) {
	if(reference==0 || reference>_references.size()) {
		ERROR("No AMF reference to repeat");
		return false;
	}
	--reference;
	if(!_amf3)
		packet.write8(AMF_AVMPLUS_OBJECT);
	packet.write8(_references[reference]);
	packet.write8(reference<<1);
	return true;
}

void AMFWriter::writeString(const string& value) {
	_lastReference=0;
	if(!_amf3) {
		if(amf0Preference) {
			if(value.size()>65535) {
				packet.write8(AMF_LONG_STRING);
				packet.write32(value.size());
				packet.writeRaw(value);
			} else {
				packet.write8(AMF_STRING);
				packet.writeString16(value);
			}
			return;
		}
		packet.write8(AMF_AVMPLUS_OBJECT);
	}
	packet.write8(AMF3_STRING); // marker
	writeText(value);
	_lastReference=_references.size();
}

void AMFWriter::writePropertyName(const string& value) {
	_lastReference=0;
	// no marker, no string_long, no empty value
	if(!_amf3) {
		packet.writeString16(value);
		return;
	}
	writeText(value);
}

void AMFWriter::writeText(const string& value) {
	if(!value.empty()) {
		map<string,UInt32>::iterator it = _stringReferences.lower_bound(value);
		if(it!=_stringReferences.end() && it->first==value) {
			packet.write7BitValue(it->second<<1);
			return;
		}
		if(it!=_stringReferences.begin())
			--it;
		_stringReferences.insert(it,pair<string,UInt32>(value,_stringReferences.size()));
	}
	packet.write7BitValue((value.size()<<1) | 0x01);
	packet.writeRaw(value);
}

void AMFWriter::writeNull() {
	_lastReference=0;
	packet.write8(_amf3 ? AMF3_NULL : AMF_NULL); // marker
}

void AMFWriter::writeBoolean(bool value){
	_lastReference=0;
	if(!_amf3) {
		packet.write8(AMF_BOOLEAN); // marker
		packet.writeBool(value);
	} else
		packet.write8(value ? AMF3_TRUE : AMF3_FALSE);
}

void AMFWriter::writeDate(const Date& date){
	_lastReference=0;
	if(!_amf3) {
		if(amf0Preference) {
			packet.write8(AMF_DATE);
			packet.writeNumber<double>((double)date);
			packet.write16(0); // Timezone, useless in AMF0 format (always equals 0)
			return;
		}
		packet.write8(AMF_AVMPLUS_OBJECT);
	}
	packet.write8(AMF3_DATE);
	packet.write8(0x01);
	packet.writeNumber<double>((double)date);
	_references.emplace_back(AMF3_DATE);
	_lastReference=_references.size();
}

void AMFWriter::writeNumber(double value){
	_lastReference=0;
	if(!amf0Preference && value<=AMF_MAX_INTEGER && round(value) == value) {
		writeInteger((Int32)value);
		return;
	}
	packet.write8(_amf3 ? AMF3_NUMBER : AMF_NUMBER); // marker
	packet.writeNumber<double>(value);
}

void AMFWriter::writeInteger(Int32 value){
	if(!_amf3)
		packet.write8(AMF_AVMPLUS_OBJECT);
	packet.write8(AMF3_INTEGER); // marker
	if(value>AMF_MAX_INTEGER) {
		ERROR("AMF Integer maximum value reached");
		value=AMF_MAX_INTEGER;
	} else if(value<0)
		value+=(1<<29);
	packet.write7BitValue(value);
}

void AMFWriter::writeBytes(const UInt8* data,UInt32 size) {
	if(!_amf3)
		packet.write8(AMF_AVMPLUS_OBJECT); // switch in AMF3 format 
	packet.write8(AMF3_BYTEARRAY); // bytearray in AMF3 format!
	packet.write7BitValue((size << 1) | 1);
	_references.emplace_back(AMF3_BYTEARRAY);
	packet.writeRaw(data,size);
	_lastReference=_references.size();
}

void AMFWriter::beginObject(const string& type,bool external) {
	_lastReference=0;
	if(!_amf3) {
		if(amf0Preference && !external) {
			_lastObjectReferences.emplace_back(0,true);
			if(type.empty())
				packet.write8(AMF_BEGIN_OBJECT);
			else {
				packet.write8(AMF_BEGIN_TYPED_OBJECT);
				writeString(type);
			}
			return;
		}
		packet.write8(AMF_AVMPLUS_OBJECT);
		_amf3=true;
	}
	packet.write8(AMF3_OBJECT);
	_references.emplace_back(AMF3_OBJECT);
	_lastReference=_references.size();
	_lastObjectReferences.emplace_back(_lastReference,!external);


	UInt32 flags = 1; // inner object
	
	// ClassDef always inline (because never hard properties, all is dynamic)
	// _references.emplace_back(0);
	flags|=(1<<1);
	if(external)
		flags|=(1<<2);
	else
		flags|=(1<<3); // Always dynamic, but can't be externalizable AND dynamic!

	/* Can be ignored finally!
	if(externalizable) {
		// What follows is the value of the “inner” object
	} else if(hardProperties>0 && !pClassDef) {
		// The remaining integer-data represents the number of class members that exist.
		// If there is a class-def reference there are no property names and the number of values is equal to the number of properties in the class-def
		flags |= (hardProperties<<4);
	}*/

	packet.write7BitValue(flags);
	writePropertyName(type);
}

void AMFWriter::beginObjectArray(UInt32 size) {
	_lastReference=0;
	if(!_amf3) {
		packet.write8(AMF_AVMPLUS_OBJECT);
		_amf3=true;
	}
	packet.write8(AMF3_ARRAY);
	packet.write7BitValue((size << 1) | 1);
	_references.emplace_back(AMF3_ARRAY);
	_lastReference=_references.size();
	_lastObjectReferences.emplace_back(_lastReference,false);
	_lastObjectReferences.emplace_back(_lastReference,false);
}

void AMFWriter::beginMap(UInt32 size,bool weakKeys) {
	_lastReference=0;
	if(!_amf3) {
		packet.write8(AMF_AVMPLUS_OBJECT);
		_amf3=true;
	}
	packet.write8(AMF3_DICTIONARY);
	packet.write7BitValue((size << 1) | 1);
	packet.write8(weakKeys ? 0x01 : 0x00);
	_references.emplace_back(AMF3_DICTIONARY);
	_lastReference=_references.size();
	_lastObjectReferences.emplace_back(_lastReference,false);
}

void AMFWriter::endObject() {
	if(_lastObjectReferences.empty()) {
		ERROR("endObject called without beginObject calling");
		return;
	}
	ObjectRef& objectRef =_lastObjectReferences.back();
	_lastReference = objectRef.reference;
	bool isObject = objectRef.isObject;
	_lastObjectReferences.pop_back();
	if(isObject && !_amf3) {
		packet.write16(0); 
		packet.write8(AMF_END_OBJECT);
		return;
	}
	packet.write8(AMF3_NULL);
	if(_lastObjectReferences.empty())
		_amf3=false;
}


} // namespace Mona
