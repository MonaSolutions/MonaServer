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

#include "Mona/AMFReader.h"
#include "Mona/Logs.h"
#include "Mona/Exceptions.h"

using namespace std;

namespace Mona {

class ObjectDef : virtual Object {
public:
	ObjectDef(UInt32 amf3,UInt8 arrayType=0) : amf3(amf3),reset(0),dynamic(false),externalizable(false),count(0),arrayType(arrayType) {}

	deque<string>	hardProperties;
	UInt32			reset;
	bool			dynamic;
	bool			externalizable;
	UInt32			count;
	UInt8			arrayType;
	const UInt32	amf3;
};


AMFReader::AMFReader(MemoryReader& reader) : DataReader(reader),_amf3(0),_amf0Reset(0),_referencing(true) {

}


AMFReader::~AMFReader() {
	for(ObjectDef* pObjectDef : _objectDefs)
		delete pObjectDef;
}

void AMFReader::reset() {
	DataReader::reset();
	for(ObjectDef* pObjectDef : _objectDefs)
		delete pObjectDef;
	_objectDefs.clear();
	_stringReferences.clear();
	_classDefReferences.clear();
	_references.clear();
	_amf0References.clear();
	_amf0Reset=0;
	_amf3 = 0;
	_referencing = true;
}

void AMFReader::readNull() {
	Type type = followingType();
	if(type==NIL) {
		reader.next(1);
		return;
	}
	ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Null type");
}

double AMFReader::readNumber() {
	Type type = followingType();
	if(type==NIL) {
		reader.next(1);
		return 0;
	}
	if(type!=NUMBER) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Number type");
		return 0;
	}
	UInt8 byte = current();
	reader.next(1);
	if(byte==AMF3_INTEGER) {
			// Forced in AMF3 here!
		UInt32 value = reader.read7BitValue();
		if(value>AMF_MAX_INTEGER)
			value-=(1<<29);
		return value;
	}
	return reader.readNumber<double>();
}

bool AMFReader::readBoolean() {
	Type type = followingType();
	if(type==NIL) {
		reader.next(1);
		return false;
	}
	if(type!=BOOLEAN) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Boolean type");
		return false;
	}
	if(_amf3)
		return reader.read8()== AMF3_FALSE ? false : true;
	reader.next(1);
	return reader.read8()==0x00 ? false : true;
}

const UInt8* AMFReader::readBytes(UInt32& size) {
	Type type = followingType();
	if(type==NIL) {
		reader.next(1);
		return NULL;
	}
	if(type!=BYTES) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF ByteArray type");
		return NULL;
	}
	reader.next(1);

	// Forced in AMF3 here!
	UInt32 reference = reader.position();
	size = reader.read7BitValue();
	bool isInline = size&0x01;
	size >>= 1;
	UInt32 reset = reader.position()+size;
	if(isInline) {
		if(_referencing)
			_references.push_back(reference);
	} else {
		if(size>_references.size()) {
			ERROR("AMF3 reference not found")
			return NULL;
		}
		reset -= size;
		reader.reset(_references[size]);
		size = reader.read7BitValue()>>1;
		reset+=size;
	}
	const UInt8* result = reader.current();
	reader.reset(reset);
	return result;
}

Time& AMFReader::readTime(Time& time) {
	Type type = followingType();
	if(type==NIL) {
		reader.next(1);
		return time.update(0);
	}
	if(type!=TIME) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Date type");
		return time.update(0);
	}

	reader.next(1);
	double result = 0;
	if (_amf3) {
		UInt32 flags = reader.read7BitValue();
		UInt32 reference = reader.position();
		bool isInline = flags & 0x01;
		if (isInline) {
			if (_referencing)
				_references.push_back(reference);
			result = reader.readNumber<double>();
		} else {
			flags >>= 1;
			if (flags > _references.size()) {
				ERROR("AMF3 reference not found")
				return time.update(0);
			}
			UInt32 reset = reader.position();
			reader.reset(_references[flags]);
			result = reader.readNumber<double>();
			reader.reset(reset);
		}
		return time.update((Int64)result * 1000);
	}
	result = reader.readNumber<double>();
	reader.next(2); // Timezone, useless
	return time.update((Int64)result * 1000);
}

string& AMFReader::readString(string& value) {
	Type type = followingType();
	if(type==NIL) {
		reader.next(1);
		value="";
		return value;
	}
	if(type!=STRING) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF String type");
		return value;
	}
	reader.next(1);
	if(_amf3)
		return readText(value);
	if(current()==AMF_LONG_STRING)
		return reader.readRaw(reader.read32(), value);
	return reader.readString16(value);
}

bool AMFReader::readMap(UInt32& size,bool& weakKeys) {
	Type type = followingType();
	if(type==NIL) {
		reader.next(1);
		return false;
	}
	if(type!=MAP) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Dictionary type");
		return false;
	}

	// AMF3
	reader.next(1); // marker
	UInt32 reference = reader.position();
	UInt32 count = reader.read7BitValue();
	bool isInline = count&0x01;
	count >>= 1;

	if(!isInline && count>_references.size()) {
		ERROR("AMF3 reference not found")
		return false;
	}

	ObjectDef* pObjectDef = new ObjectDef(_amf3,AMF3_DICTIONARY);
	pObjectDef->dynamic=true;
	_objectDefs.push_back(pObjectDef);
	
	if(isInline) {
		if(_referencing)
			_references.push_back(reference);
		pObjectDef->count = size = count;
	} else {
		pObjectDef->reset = reader.position();
		reader.reset(_references[count]);
		pObjectDef->count = size = reader.read7BitValue()>>1;
	}
	pObjectDef->count *= 2;
	weakKeys = reader.read8()&0x01;

	return true;
}

AMFReader::Type AMFReader::readKey() {
	if(_objectDefs.size()==0) {
		ERROR("readKey/readValue called without a readMap before");
		return END;
	}

	ObjectDef& objectDef = *_objectDefs.back();
	_amf3 = objectDef.amf3;

	if(objectDef.arrayType != AMF3_DICTIONARY) {
		ERROR("readKey/readValue must be called after a readMap");
		return END;
	}

	if(objectDef.count==0) {
		if(objectDef.reset)
			reader.reset(objectDef.reset);
		delete &objectDef;
		_objectDefs.pop_back();
		return END;
	}
	--objectDef.count;
	return followingType();
}

bool AMFReader::readArray(UInt32& size) {
	Type type = followingType();
	if(type==NIL) {
		reader.next(1);
		return false;
	}
	if(type!=ARRAY) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Array type");
		return false;
	}

	if(!_amf3) {
		ObjectDef* pObjectDef = new ObjectDef(_amf3,current());
		_objectDefs.push_back(pObjectDef);
		pObjectDef->dynamic=true;
		if(_referencing)
			_amf0References.push_back(reader.position());
		if(_amf0Reset)
			pObjectDef->reset = _amf0Reset;
		reader.next(1);
		size = reader.read32();
		if(pObjectDef->arrayType==AMF_STRICT_ARRAY)
			pObjectDef->count = size;
		return true;
	}
	
	// AMF3
	reader.next(1); // marker
	UInt32 reference = reader.position();
	UInt32 count = reader.read7BitValue();
	bool isInline = count&0x01;
	count >>= 1;

	if(!isInline && count>_references.size()) {
		ERROR("AMF3 reference not found")
		return false;
	}

	ObjectDef* pObjectDef = new ObjectDef(_amf3,AMF3_ARRAY);
	pObjectDef->dynamic=true;
	_objectDefs.push_back(pObjectDef);
	
	if(isInline) {
		if(_referencing)
			_references.push_back(reference);
		 pObjectDef->count = size = count;
	} else {
		pObjectDef->reset = reader.position();
		reader.reset(_references[size]);
		pObjectDef->count = size = reader.read7BitValue()>>1;
	}

	return true;
}


bool AMFReader::readObject(string& type,bool& external) {
	Type marker = followingType();
	if(marker==NIL) {
		reader.next(1);
		return false;
	}
	if(marker!=OBJECT) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)marker)," is not a AMF Object type");
		return false;
	}

	external = false;
	if(!_amf3) {
		if(_referencing)
			_amf0References.push_back(reader.position());
		if(current()==AMF_BEGIN_TYPED_OBJECT) {
			reader.next(1);
			readText(type);
		} else
			reader.next(1);
		ObjectDef* pObjectDef = new ObjectDef(_amf3);
		_objectDefs.push_back(pObjectDef);
		if(_amf0Reset)
			pObjectDef->reset = _amf0Reset;
		pObjectDef->dynamic = true;
		return true;
	}
	
	// AMF3
	reader.next(1); // marker
	UInt32 reference = reader.position();
	UInt32 flags = reader.read7BitValue();
	bool isInline = flags&0x01;
	flags >>= 1;

	if(!isInline && flags>_references.size()) {
		ERROR("AMF3 reference not found")
		return false;
	}

	ObjectDef* pObjectDef = new ObjectDef(_amf3);
	_objectDefs.push_back(pObjectDef);
	
	if(isInline) {
		if(_referencing)
			_references.push_back(reference);
	} else {
		pObjectDef->reset = reader.position();
		reader.reset(_references[flags]);
		flags = reader.read7BitValue()>>1;
	}

	// classdef reading
	isInline = flags&0x01; 
	flags >>= 1;
	UInt32 reset=0;
	if(isInline) {
		 _classDefReferences.push_back(reference);
		readText(type);
	} else if(flags<=_classDefReferences.size()) {
		reset = reader.position();
		reader.reset(_classDefReferences[flags]);
		flags = reader.read7BitValue()>>2;
		readText(type);
	} else {
		ERROR("AMF3 classDef reference not found")
		flags=0x02; // emulate dynamic class without hard properties
	}

	if(flags&0x01)
		pObjectDef->externalizable = external = true;
	else if(flags&0x02)
		pObjectDef->dynamic=true;
	flags>>=2;

	if(!pObjectDef->externalizable) {
		pObjectDef->hardProperties.resize(flags);
		for(string& hardProperty : pObjectDef->hardProperties)
			readText(hardProperty);
	}

	if(reset>0)
 		reader.reset(reset); // reset classdef

	return true;
}

AMFReader::Type AMFReader::readItem(string& name) {
	if(_objectDefs.size()==0) {
		ERROR("readItem called without a readObject or a readArray before");
		return END;
	}

	ObjectDef& objectDef = *_objectDefs.back();
	_amf3 = objectDef.amf3;
	bool end=false;

	if(objectDef.arrayType == AMF3_DICTIONARY) {
		ERROR("readItem on a map, use readKey and readValue rather");
		return END;
	}

	if(objectDef.externalizable)
		end=true;
	else if(objectDef.hardProperties.size()>0) {
		name = objectDef.hardProperties.front();
		objectDef.hardProperties.pop_front();
	} else if(objectDef.arrayType == AMF_STRICT_ARRAY) {
		if(objectDef.count==0)
			end=true;
		else {
			--objectDef.count;
			name.clear();
		}
	} else {
		if (readText(name).empty()) {
			if(objectDef.arrayType == AMF3_ARRAY) {
				objectDef.arrayType = AMF_STRICT_ARRAY;
				return readItem(name);
			}
			end=true;
		} else if(objectDef.arrayType) {
			int index = 0;
			if (String::ToNumber<int>(name, index) && index >= 0)
				name.clear();
		}
	}

	if(end) {
		if(!_amf3 && objectDef.arrayType!=AMF_STRICT_ARRAY) {
			UInt8 marker = reader.read8();
			if(marker!=AMF_END_OBJECT)
				ERROR("AMF0 end marker object absent");
		}
		if(objectDef.reset>0)
			reader.reset(objectDef.reset);
		delete &objectDef;
		_objectDefs.pop_back();
		return END;
	}
	
	return followingType();
}


string& AMFReader::readText(string& value) {
	if(!_amf3)
		return reader.readString16(value);
	
	UInt32 reference = reader.position();
	UInt32 size = reader.read7BitValue();
	bool isInline = size&0x01;
	size >>= 1;
	if(isInline) {
		if (!reader.readRaw(size, value).empty())
			_stringReferences.push_back(reference);
	} else {
		if(size>_stringReferences.size()) {
			ERROR("AMF3 string reference not found")
			return value;
		}
		UInt32 reset = reader.position();
		reader.reset(_stringReferences[size]);
		reader.readRaw(reader.read7BitValue()>>1,value);
		reader.reset(reset);
	}
	return value;
}


AMFReader::Type AMFReader::followingType() {
	if(_amf3!=reader.position()) {
		if(_objectDefs.size()>0)
			_amf3=_objectDefs.back()->amf3;
		else
			_amf3=0;
	}
	if(!available())
		return END;
	
	UInt8 type = current();
	if(!_amf3 && type==AMF_AVMPLUS_OBJECT) {
		reader.next(1);
		_amf3=reader.position();
		if(!available())
			return END;
		type = current();
	}
	
	if(_amf3) {
		switch(type) {
			case AMF3_UNDEFINED:
			case AMF3_NULL:
				return NIL;
			case AMF3_FALSE:
			case AMF3_TRUE:
				return BOOLEAN;
			case AMF3_INTEGER:
			case AMF3_NUMBER:
				return NUMBER;
			case AMF3_STRING:
				return STRING;
			case AMF3_DATE:
				return TIME;
			case AMF3_ARRAY:
				return ARRAY;
			case AMF3_DICTIONARY:
				return MAP;
			case AMF3_OBJECT:
				return OBJECT;
			case AMF3_BYTEARRAY:
				return BYTES;
			default:
				ERROR("Unknown AMF3 type ",Format<UInt8>("%.2x",(UInt8)type))
				reader.next(1);
				return followingType();
		}
	}
	switch(type) {
		case AMF_UNDEFINED:
		case AMF_NULL:
			return NIL;
		case AMF_BOOLEAN:
			return BOOLEAN;
		case AMF_NUMBER:
			return NUMBER;
		case AMF_LONG_STRING:
		case AMF_STRING:
			return STRING;
		case AMF_MIXED_ARRAY:
		case AMF_STRICT_ARRAY:
			return ARRAY;
		case AMF_DATE:
			return TIME;
		case AMF_BEGIN_OBJECT:
		case AMF_BEGIN_TYPED_OBJECT:
			return OBJECT;
		case AMF_REFERENCE: {
			reader.next(1);
			UInt16 reference = reader.read16();
			if(reference>_amf0References.size()) {
				ERROR("AMF0 reference not found")
				return followingType();
			}
			_amf0Reset = reader.position();
			reader.reset(_amf0References[reference]);
			return followingType();
		}
		case AMF_END_OBJECT:
			ERROR("AMF end object type without begin object type before")
			reader.next(1);
			return followingType();
		case AMF_UNSUPPORTED:
			WARN("Unsupported type in AMF format")
			reader.next(1);
			return followingType();
		default:
			ERROR("Unknown AMF type ",Format<UInt8>("%.2x",(UInt8)type))
			reader.next(1);
			return followingType();
	}
}



} // namespace Mona
