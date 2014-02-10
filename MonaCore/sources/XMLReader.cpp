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

#include "Mona/XMLReader.h"
#include "Mona/Logs.h"
#include "Mona/Util.h"
#include <sstream>
#include <cstring>

using namespace std;

namespace Mona {

XMLReader::XMLReader(PacketReader& packet) : DataReader(packet),_last(NIL),_dval(0),_tagOpened(false),_objectPrimitive(false),_inited(false),_nextStep(NOTHING) {}

void XMLReader::reset() {
	packet.reset(_pos);
	_text.clear();
	_last=NIL;
	_dval=0;
	_tagOpened=false;
	_objectPrimitive=false;
	_inited=false;
	_nextStep=NOTHING;
	_queueTags.clear();
}

const UInt8* XMLReader::readBytes(UInt32& size) {
	_last=NIL;
	size = _text.size();
	return (const UInt8*)_text.c_str();
}

bool XMLReader::readBoolean() {
	_last=NIL;
	if(_last==STRING) {
		string tmp = _text;
		String::ToLower(tmp);
		if (tmp=="true")
			return true;
		else if (tmp=="false")
			return false;
	}	

	return _dval > 0;
}

Time& XMLReader::readTime(Time& time) {
	_last=NIL;
	return time.update(_date);
}

string& XMLReader::readString(string& value) {
	_last=NIL;
	value.assign(_text);
	return value;
}

double XMLReader::readNumber() {
	_last=NIL;
	return _dval;
}

bool XMLReader::readObject(string& type,bool& external) {
	_last=NIL;
	return true;
}

bool XMLReader::readArray(UInt32& size) {
	
	_last=NIL;

	if (!_inited) {
		_inited=true;
		_queueTags.emplace_back("");
	} else if (!_queueTags.empty()) {
		TagXML& tag = _queueTags.back();
		tag.arrayStarted=true;
	}

	return true;
}

XMLReader::Type XMLReader::readItem(string& name) {
	const UInt8* cur = current();
	if(!cur) {
		//ERROR("XML item absent, no more data available")
		return END;
	}
	
	// Attributes
	if (_tagOpened) {

		if (*cur=='/') {

			packet.next();
			if((cur=current()) == NULL || *cur!='>') {
				ERROR("End of tag not well formed")
				return END;
			}
			packet.next(); // escape '>'
			_queueTags.pop_back();
			_tagOpened=false;
			return END;
		} 
		else if (*cur=='>') {
			
			packet.next();
			_tagOpened=false;
		} else {

			// Read Next attribute
			UInt8* pos = (UInt8*)cur;
			UInt32 size = packet.available();
			while((size-(pos-cur))>0 && *pos!='=')
				pos++;

			if (size-(pos-cur)==0) {
				ERROR("XML malformed, tag", _text," does not terminate")
				return END;
			}
			packet.readRaw(pos-cur, name);
			packet.next(); // escape '='

			// Return type of attribute's value
			return XMLReader::followingType();
		}
	}
	
	// Read next subtag
	Type type = XMLReader::followingType();

	// Primitive type : assign name before read value
	if (type == OBJECT && _objectPrimitive) {
		name.assign(_text);
		type = XMLReader::followingType();
	}
	// Only array and root tag have a name
	else if (type == ARRAY || (type == OBJECT && (_queueTags.size()==1)))
		name.assign(_text);
	// Primitive tag value
	else if (type != OBJECT && type != END)
		name.assign("__value");

	return type;
}

XMLReader::Type XMLReader::followingType() {
	if(_last!=NIL && _last!=END)
		return _last;
	if(_dval)
		_dval=0;

	const UInt8* cur = current();
	if(!cur)
		return END;

	// First element is an array
	_last=END;
	if (!_inited) {
		_last=ARRAY;
	}
	// Terminate suspended action
	else if (_nextStep) {

		switch(_nextStep) {
		case POP_TAG:
			_nextStep=NOTHING;
			return removeTag();
			break;
		case ADD_TAG:
			_nextStep=NOTHING;
			_queueTags.emplace_back(_text);
			_tagOpened=true;
			_last = OBJECT;
			break;
		case START_ARRAY:
			_nextStep=ADD_TAG;
			_last=ARRAY;
			break;
		}
	}
	// Tag begin/end
	else if(*cur=='<') {

		if (!available()) {
			ERROR("XML malformed, tag does not terminate")
			return END;
		}
		packet.next();
		cur=current();

		// End Header/Comment
		const UInt8* first = cur;
		if (*first == '?' || *first == '!')
			return parseComment(first, cur);

		// Read Tag name
		do {
			packet.next();
		} while((cur=packet.current()) && *cur!='>' && (cur==first || *cur!='/') && !isspace(*cur));

		_text.resize(cur-first);
		memcpy((UInt8*)_text.data(), first, cur-first);

		// Got to next char
		cur=current();

		if (cur==NULL) {
			ERROR("XML malformed, tag ", _text, "does not terminate")
			return END;
		}

		// End tag
		if (*first == '/') {
			_text.erase(_text.begin()); // Erase '/'

			if (_queueTags.empty()) {
				ERROR("End of tag ", _text, " without start")
				return END;
			}

			// End of a parent tag
			TagXML& tag = _queueTags.back();
			if (tag.tagName==_text && !tag.currentSubTag.empty()) {
				tag.currentSubTag.assign("");
				_nextStep=POP_TAG;
				return END; // First : End of array
			}
			return removeTag();
		}
		// If no attributes and no tag : it is a primitive tag
		else if (*cur=='>' && (isalnum(*(cur+1)) || *(cur+1)=='-' || *(cur+1)=='\"')) {
			
			// TODO see if we manage <tag></tag>
			packet.next();
			_objectPrimitive=true;
			return OBJECT;
		} 
		// New subtag
		else if (!_queueTags.empty()) {

			TagXML& tag = _queueTags.back();
			// First subTag
			if (tag.currentSubTag.empty())
				tag.currentSubTag.assign(_text);
			else if (tag.currentSubTag != _text) {
				tag.currentSubTag.assign(_text);
				tag.arrayStarted=false;
				_nextStep=START_ARRAY;
				return END; // End of last Array
			}

			// First of all : create Array of subtags
			if(!tag.arrayStarted) {
				_nextStep=ADD_TAG;
				_last=ARRAY;
				return ARRAY;
			}
		}

		// New tag
		_queueTags.emplace_back(_text);
		_tagOpened=true;
		_last = OBJECT;
	}
	// Value
	else if(isalnum(*cur) || *cur=='-' || *cur=='\"')
		return parsePrimitive(cur);

	return _last;
}

XMLReader::Type XMLReader::parseComment(const UInt8* first, const UInt8* cur) {

	bool finished = false;
	do {
		packet.next(1);
		cur=packet.current();

		if (cur && *first == '?' && *cur=='>' && *(cur-1)=='?') // ?>
			finished=true;
		else if (cur && *cur=='>' && *(cur-1)=='-' && *(cur-2)=='-') // -->
			finished=true;
	} while((cur!=NULL) && !finished);

	if(!available())
		return END;

	// go to next object
	packet.next(1);
	return XMLReader::followingType();
}

XMLReader::Type XMLReader::parsePrimitive(const UInt8* cur) {

	bool chained=false;
	if (*cur=='\"') {
		chained=true;
		packet.next(1);
	}
	
	// TODO Treat CDATA
	const UInt8* first = packet.current();
	while((cur=current()) && *cur!='\"' && *cur!='<')
		packet.next(1);

	if(!cur) {
		ERROR("XML malformed, tag does not terminate");
		return END;
	}
	if(chained != (*cur=='\"')) {
		ERROR("XML malformed, character '\"' founded inappropriately");
		return END;
	}

	_text.resize(cur-first);
	memcpy((UInt8*)_text.data(), first, cur-first);
	String::Trim(_text);

	_last=STRING;
	Exception ex;
	_dval = String::ToNumber<double>(ex, _text);
	if (!ex) 
		_last = NUMBER;
	else if (_text.size() > 18 && _text.size() < 34 && _date.fromString(_text))
		_last=TIME;

	if(chained)
		packet.next(1); // remove last '"'

	return _last;
}

XMLReader::Type XMLReader::removeTag() {

	packet.next();
	_queueTags.pop_back();

	// For primitive we must ignore end tag
	if (_objectPrimitive) {
		_objectPrimitive=false;
		return XMLReader::followingType();
	}
	return END;
}

const UInt8* XMLReader::current() {
	while(available() && isspace(*packet.current()))
		packet.next(1);
	if(!available())
		return NULL;
	return packet.current();
}

} // namespace Mona
