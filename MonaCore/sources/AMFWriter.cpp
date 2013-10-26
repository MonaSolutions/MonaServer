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

#include "Mona/AMFWriter.h"
#include "Mona/Logs.h"
#include "math.h"

using namespace std;
using namespace Poco;

namespace Mona {

class ObjectRef : virtual Object {
public:
	ObjectRef(UInt32 reference,bool isObject) : reference(reference),isObject(isObject) {}

	const UInt32 reference;
	const bool isObject;
};

AMFWriter::AMFWriter() : _amf3(false),amf0Preference(false) {

}

AMFWriter::~AMFWriter() {
	if(!_lastObjectReferences.empty())
		ERROR("AMFWriter end without terminating a complex type (object, map or array)");
	list<ObjectRef*>::iterator it;
	for(it=_lastObjectReferences.begin();it!=_lastObjectReferences.end();++it)
		delete *it;
}

void AMFWriter::clear() {
	_amf3=false;
	amf0Preference=false;
	list<ObjectRef*>::iterator it;
	for(it=_lastObjectReferences.begin();it!=_lastObjectReferences.end();++it)
		delete *it;
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
		writer.write8(AMF_AVMPLUS_OBJECT);
	writer.write8(_references[reference]);
	writer.write8(reference<<1);
	return true;
}

void AMFWriter::writeString(const string& value) {
	_lastReference=0;
	if(!_amf3) {
		if(amf0Preference) {
			if(value.size()>65535) {
				writer.write8(AMF_LONG_STRING);
				writer.write32(value.size());
				writer.writeRaw(value);
			} else {
				writer.write8(AMF_STRING);
				writer.writeString16(value);
			}
			return;
		}
		writer.write8(AMF_AVMPLUS_OBJECT);
	}
	writer.write8(AMF3_STRING); // marker
	writeText(value);
	_lastReference=_references.size();
}

void AMFWriter::writePropertyName(const string& value) {
	_lastReference=0;
	// no marker, no string_long, no empty value
	if(!_amf3) {
		writer.writeString16(value);
		return;
	}
	writeText(value);
}

void AMFWriter::writeText(const string& value) {
	if(!value.empty()) {
		map<string,UInt32>::iterator it = _stringReferences.lower_bound(value);
		if(it!=_stringReferences.end() && it->first==value) {
			writer.write7BitValue(it->second<<1);
			return;
		}
		if(it!=_stringReferences.begin())
			--it;
		_stringReferences.insert(it,pair<string,UInt32>(value,_stringReferences.size()));
	}
	writer.write7BitValue((value.size()<<1) | 0x01);
	writer.writeRaw(value);
}

void AMFWriter::writeNull() {
	_lastReference=0;
	writer.write8(_amf3 ? AMF3_NULL : AMF_NULL); // marker
}

void AMFWriter::writeBoolean(bool value){
	_lastReference=0;
	if(!_amf3) {
		writer.write8(AMF_BOOLEAN); // marker
		writer << value;
	} else
		writer.write8(value ? AMF3_TRUE : AMF3_FALSE);
}

void AMFWriter::writeDate(const Time& date){
	_lastReference=0;
	if(!_amf3) {
		if(amf0Preference) {
			writer.write8(AMF_DATE);
			writer << ((double)date/1000);
			writer.write16(0); // Timezone, useless in AMF0 format (always equals 0)
			return;
		}
		writer.write8(AMF_AVMPLUS_OBJECT);
	}
	writer.write8(AMF3_DATE);
	writer.write8(0x01);
	writer << ((double)date/1000);
	_references.push_back(AMF3_DATE);
	_lastReference=_references.size();
}

void AMFWriter::writeNumber(double value){
	_lastReference=0;
	if(!amf0Preference && value<=AMF_MAX_INTEGER && ROUND(value) == value) {
		writeInteger((Int32)value);
		return;
	}
	writer.write8(_amf3 ? AMF3_NUMBER : AMF_NUMBER); // marker
	writer << value;
}

void AMFWriter::writeInteger(Int32 value){
	if(!_amf3)
		writer.write8(AMF_AVMPLUS_OBJECT);
	writer.write8(AMF3_INTEGER); // marker
	if(value>AMF_MAX_INTEGER) {
		ERROR("AMF Integer maximum value reached");
		value=AMF_MAX_INTEGER;
	} else if(value<0)
		value+=(1<<29);
	writer.write7BitValue(value);
}

void AMFWriter::writeBytes(const UInt8* data,UInt32 size) {
	if(!_amf3)
		writer.write8(AMF_AVMPLUS_OBJECT); // switch in AMF3 format 
	writer.write8(AMF3_BYTEARRAY); // bytearray in AMF3 format!
	writer.write7BitValue((size << 1) | 1);
	_references.push_back(AMF3_BYTEARRAY);
	writer.writeRaw(data,size);
	_lastReference=_references.size();
}

void AMFWriter::beginObject(const string& type,bool external) {
	_lastReference=0;
	if(!_amf3) {
		if(amf0Preference && !external) {
			_lastObjectReferences.push_back(new ObjectRef(0,true));
			if(type.empty())
				writer.write8(AMF_BEGIN_OBJECT);
			else {
				writer.write8(AMF_BEGIN_TYPED_OBJECT);
				writeString(type);
			}
			return;
		}
		writer.write8(AMF_AVMPLUS_OBJECT);
		_amf3=true;
	}
	writer.write8(AMF3_OBJECT);
	_references.push_back(AMF3_OBJECT);
	_lastReference=_references.size();
	_lastObjectReferences.push_back(new ObjectRef(_lastReference,!external));


	UInt32 flags = 1; // inner object
	
	// ClassDef always inline (because never hard properties, all is dynamic)
	// _references.push_back(0);
	flags|=(1<<1);
	if(external)
		flags|=(1<<2);
	else
		flags|=(1<<3); // Always dynamic, but can't be externalizable AND dynamic!


	/* TODO?
	if(externalizable) {
		// What follows is the value of the “inner” object
	} else if(hardProperties>0 && !pClassDef) {
		// The remaining integer-data represents the number of class members that exist.
		// If there is a class-def reference there are no property names and the number of values is equal to the number of properties in the class-def
		flags |= (hardProperties<<4);
	}*/

	writer.write7BitValue(flags);
	writePropertyName(type);
}

void AMFWriter::beginObjectArray(UInt32 size) {
	_lastReference=0;
	if(!_amf3) {
		writer.write8(AMF_AVMPLUS_OBJECT);
		_amf3=true;
	}
	writer.write8(AMF3_ARRAY);
	writer.write7BitValue((size << 1) | 1);
	_references.push_back(AMF3_ARRAY);
	_lastReference=_references.size();
	_lastObjectReferences.push_back(new ObjectRef(_lastReference,false));
	_lastObjectReferences.push_back(new ObjectRef(_lastReference,false));
}

void AMFWriter::beginMap(UInt32 size,bool weakKeys) {
	_lastReference=0;
	if(!_amf3) {
		writer.write8(AMF_AVMPLUS_OBJECT);
		_amf3=true;
	}
	writer.write8(AMF3_DICTIONARY);
	writer.write7BitValue((size << 1) | 1);
	writer.write8(weakKeys ? 0x01 : 0x00);
	_references.push_back(AMF3_DICTIONARY);
	_lastReference=_references.size();
	_lastObjectReferences.push_back(new ObjectRef(_lastReference,false));
}

void AMFWriter::endObject() {
	if(_lastObjectReferences.size()==0) {
		ERROR("endObject called without beginObject calling");
		return;
	}
	ObjectRef* pObjectRef =_lastObjectReferences.back();
	_lastReference = pObjectRef->reference;
	bool isObject = pObjectRef->isObject;
	delete pObjectRef;
	_lastObjectReferences.pop_back();
	if(isObject) {
		if(!_amf3) {
			writer.write16(0); 
			writer.write8(AMF_END_OBJECT);
			return;
		}
		writer.write8(01); // end marker
	}
	if(_lastObjectReferences.size()==0 || _lastObjectReferences.back()==0)
		_amf3=false;
}


} // namespace Mona
