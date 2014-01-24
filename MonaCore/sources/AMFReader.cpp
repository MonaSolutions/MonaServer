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



AMFReader::AMFReader(PacketReader& packet) : DataReader(packet),_amf3(0),_amf0Reset(0),_referencing(true) {

}


void AMFReader::reset() {
	DataReader::reset();
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
		packet.next(1);
		return;
	}
	ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Null type");
}

double AMFReader::readNumber() {
	Type type = followingType();
	if(type==NIL) {
		packet.next(1);
		return 0;
	}
	if(type!=NUMBER) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Number type");
		return 0;
	}
	UInt8 byte = current();
	packet.next(1);
	if(byte==AMF3_INTEGER) {
			// Forced in AMF3 here!
		UInt32 value = packet.read7BitValue();
		if(value>AMF_MAX_INTEGER)
			value-=(1<<29);
		return value;
	}
	return packet.readNumber<double>();
}

bool AMFReader::readBoolean() {
	Type type = followingType();
	if(type==NIL) {
		packet.next(1);
		return false;
	}
	if(type!=BOOLEAN) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Boolean type");
		return false;
	}
	if(_amf3)
		return packet.read8()== AMF3_FALSE ? false : true;
	packet.next(1);
	return packet.read8()==0x00 ? false : true;
}

const UInt8* AMFReader::readBytes(UInt32& size) {
	Type type = followingType();
	if(type==NIL) {
		packet.next(1);
		return NULL;
	}
	if(type!=BYTES) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF ByteArray type");
		return NULL;
	}
	packet.next(1);

	// Forced in AMF3 here!
	UInt32 reference = packet.position();
	size = packet.read7BitValue();
	bool isInline = size&0x01;
	size >>= 1;
	UInt32 reset = packet.position()+size;
	if(isInline) {
		if(_referencing)
			_references.emplace_back(reference);
	} else {
		if(size>_references.size()) {
			ERROR("AMF3 reference not found")
			return NULL;
		}
		reset -= size;
		packet.reset(_references[size]);
		size = packet.read7BitValue()>>1;
		reset+=size;
	}
	const UInt8* result = packet.current();
	packet.reset(reset);
	return result;
}

Time& AMFReader::readTime(Time& time) {
	Type type = followingType();
	if(type==NIL) {
		packet.next(1);
		return time.update(0);
	}
	if(type!=TIME) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Date type");
		return time.update(0);
	}

	packet.next(1);
	double result = 0;
	if (_amf3) {
		UInt32 flags = packet.read7BitValue();
		UInt32 reference = packet.position();
		bool isInline = flags & 0x01;
		if (isInline) {
			if (_referencing)
				_references.emplace_back(reference);
			result = packet.readNumber<double>();
		} else {
			flags >>= 1;
			if (flags > _references.size()) {
				ERROR("AMF3 reference not found")
				return time.update(0);
			}
			UInt32 reset = packet.position();
			packet.reset(_references[flags]);
			result = packet.readNumber<double>();
			packet.reset(reset);
		}
		return time.update((Int64)result * 1000);
	}
	result = packet.readNumber<double>();
	packet.next(2); // Timezone, useless
	return time.update((Int64)result * 1000);
}

string& AMFReader::readString(string& value) {
	Type type = followingType();
	if(type==NIL) {
		packet.next(1);
		value="";
		return value;
	}
	if(type!=STRING) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF String type");
		return value;
	}
	packet.next(1);
	if(_amf3)
		return readText(value);
	if(current()==AMF_LONG_STRING)
		return packet.readRaw(packet.read32(), value);
	return packet.readString16(value);
}

bool AMFReader::readMap(UInt32& size,bool& weakKeys) {
	Type type = followingType();
	if(type==NIL) {
		packet.next(1);
		return false;
	}
	if(type!=MAP) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Dictionary type");
		return false;
	}

	// AMF3
	packet.next(1); // marker
	UInt32 reference = packet.position();
	UInt32 count = packet.read7BitValue();
	bool isInline = count&0x01;
	count >>= 1;

	if(!isInline && count>_references.size()) {
		ERROR("AMF3 reference not found")
		return false;
	}

	_objectDefs.emplace_back(_amf3,AMF3_DICTIONARY);
	ObjectDef& objectDef = _objectDefs.back();
	objectDef.dynamic=true;
	
	if(isInline) {
		if(_referencing)
			_references.emplace_back(reference);
		objectDef.count = size = count;
	} else {
		objectDef.reset = packet.position();
		packet.reset(_references[count]);
		objectDef.count = size = packet.read7BitValue()>>1;
	}
	objectDef.count *= 2;
	weakKeys = packet.read8()&0x01;

	return true;
}

AMFReader::Type AMFReader::readKey() {
	if(_objectDefs.empty()) {
		ERROR("readKey/readValue called without a readMap before");
		return END;
	}

	ObjectDef& objectDef = _objectDefs.back();
	_amf3 = objectDef.amf3;

	if(objectDef.arrayType != AMF3_DICTIONARY) {
		ERROR("readKey/readValue must be called after a readMap");
		return END;
	}

	if(objectDef.count==0) {
		if(objectDef.reset)
			packet.reset(objectDef.reset);
		_objectDefs.pop_back();
		return END;
	}
	--objectDef.count;
	return followingType();
}

bool AMFReader::readArray(UInt32& size) {
	Type type = followingType();
	if(type==NIL) {
		packet.next(1);
		return false;
	}
	if(type!=ARRAY) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)type)," is not a AMF Array type");
		return false;
	}

	if(!_amf3) {
		_objectDefs.emplace_back(_amf3,current());
		ObjectDef& objectDef = _objectDefs.back();
		objectDef.dynamic=true;
		if(_referencing)
			_amf0References.emplace_back(packet.position());
		if(_amf0Reset)
			objectDef.reset = _amf0Reset;
		packet.next(1);
		size = packet.read32();
		if(objectDef.arrayType==AMF_STRICT_ARRAY)
			objectDef.count = size;
		return true;
	}
	
	// AMF3
	packet.next(1); // marker
	UInt32 reference = packet.position();
	UInt32 count = packet.read7BitValue();
	bool isInline = count&0x01;
	count >>= 1;

	if(!isInline && count>_references.size()) {
		ERROR("AMF3 reference not found")
		return false;
	}

	_objectDefs.emplace_back(_amf3,AMF3_ARRAY);
	ObjectDef& objectDef = _objectDefs.back();
	objectDef.dynamic = true;
	
	if(isInline) {
		if(_referencing)
			_references.emplace_back(reference);
		 objectDef.count = size = count;
	} else {
		objectDef.reset = packet.position();
		packet.reset(_references[size]);
		objectDef.count = size = packet.read7BitValue()>>1;
	}

	return true;
}


bool AMFReader::readObject(string& type,bool& external) {
	Type marker = followingType();
	if(marker==NIL) {
		packet.next(1);
		return false;
	}
	if(marker!=OBJECT) {
		ERROR("Type ",Format<UInt8>("%.2x",(UInt8)marker)," is not a AMF Object type");
		return false;
	}

	external = false;
	if(!_amf3) {
		if(_referencing)
			_amf0References.emplace_back(packet.position());
		if(current()==AMF_BEGIN_TYPED_OBJECT) {
			packet.next(1);
			readText(type);
		} else
			packet.next(1);
		_objectDefs.emplace_back(_amf3);
		ObjectDef& objectDef = _objectDefs.back();
		if(_amf0Reset)
			objectDef.reset = _amf0Reset;
		objectDef.dynamic = true;
		return true;
	}
	
	// AMF3
	packet.next(1); // marker
	UInt32 reference = packet.position();
	UInt32 flags = packet.read7BitValue();
	bool isInline = flags&0x01;
	flags >>= 1;

	if(!isInline && flags>_references.size()) {
		ERROR("AMF3 reference not found")
		return false;
	}

	_objectDefs.emplace_back(_amf3);
	ObjectDef& objectDef = _objectDefs.back();
	
	if(isInline) {
		if(_referencing)
			_references.emplace_back(reference);
	} else {
		objectDef.reset = packet.position();
		packet.reset(_references[flags]);
		flags = packet.read7BitValue()>>1;
	}

	// classdef reading
	isInline = flags&0x01; 
	flags >>= 1;
	UInt32 reset=0;
	if(isInline) {
		 _classDefReferences.emplace_back(reference);
		readText(type);
	} else if(flags<=_classDefReferences.size()) {
		reset = packet.position();
		packet.reset(_classDefReferences[flags]);
		flags = packet.read7BitValue()>>2;
		readText(type);
	} else {
		ERROR("AMF3 classDef reference not found")
		flags=0x02; // emulate dynamic class without hard properties
	}

	if(flags&0x01)
		objectDef.externalizable = external = true;
	else if(flags&0x02)
		objectDef.dynamic=true;
	flags>>=2;

	if(!objectDef.externalizable) {
		objectDef.hardProperties.resize(flags);
		for(string& hardProperty : objectDef.hardProperties)
			readText(hardProperty);
	}

	if(reset>0)
 		packet.reset(reset); // reset classdef

	return true;
}

AMFReader::Type AMFReader::readItem(string& name) {
	if(_objectDefs.empty()) {
		ERROR("readItem called without a readObject or a readArray before");
		return END;
	}

	ObjectDef& objectDef = _objectDefs.back();
	_amf3 = objectDef.amf3;
	bool end=false;

	if(objectDef.arrayType == AMF3_DICTIONARY) {
		ERROR("readItem on a map, use readKey and readValue rather");
		return END;
	}

	if(objectDef.externalizable)
		end=true;
	else if(!objectDef.hardProperties.empty()) {
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
			UInt8 marker = packet.read8();
			if(marker!=AMF_END_OBJECT)
				ERROR("AMF0 end marker object absent");
		}
		if(objectDef.reset>0)
			packet.reset(objectDef.reset);
		_objectDefs.pop_back();
		return END;
	}
	
	return followingType();
}


string& AMFReader::readText(string& value) {
	if(!_amf3)
		return packet.readString16(value);
	
	UInt32 reference = packet.position();
	UInt32 size = packet.read7BitValue();
	bool isInline = size&0x01;
	size >>= 1;
	if(isInline) {
		if (!packet.readRaw(size, value).empty())
			_stringReferences.emplace_back(reference);
	} else {
		if(size>_stringReferences.size()) {
			ERROR("AMF3 string reference not found")
			return value;
		}
		UInt32 reset = packet.position();
		packet.reset(_stringReferences[size]);
		packet.readRaw(packet.read7BitValue()>>1,value);
		packet.reset(reset);
	}
	return value;
}


AMFReader::Type AMFReader::followingType() {
	if(_amf3!=packet.position()) {
		if(!_objectDefs.empty())
			_amf3=_objectDefs.back().amf3;
		else
			_amf3=0;
	}
	if(!available())
		return END;
	
	UInt8 type = current();
	if(!_amf3 && type==AMF_AVMPLUS_OBJECT) {
		packet.next(1);
		_amf3=packet.position();
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
				packet.next(1);
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
			packet.next(1);
			UInt16 reference = packet.read16();
			if(reference>_amf0References.size()) {
				ERROR("AMF0 reference not found")
				return followingType();
			}
			_amf0Reset = packet.position();
			packet.reset(_amf0References[reference]);
			return followingType();
		}
		case AMF_END_OBJECT:
			ERROR("AMF end object type without begin object type before")
			packet.next(1);
			return followingType();
		case AMF_UNSUPPORTED:
			WARN("Unsupported type in AMF format")
			packet.next(1);
			return followingType();
		default:
			ERROR("Unknown AMF type ",Format<UInt8>("%.2x",(UInt8)type))
			packet.next(1);
			return followingType();
	}
}



} // namespace Mona
