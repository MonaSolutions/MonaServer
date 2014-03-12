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

using namespace std;

namespace Mona {

bool XMLReader::isValid() {

	const UInt8* cur = current();

	return cur!=NULL && *cur=='<';
}

XMLReader::XMLReader(PacketReader& packet) : DataReader(packet),_last(NIL),_dval(0),_tagOpened(false),_isProperty(false),_nextStep(NOTHING),_nextType(NIL) {

	if (!isValid())
		return;

	// Ignore first '<__array>' and last '</__array>'
	if(followingType()==ARRAY) {
		if(readArray(_pos) && packet.available()>0) {

			// Search for /__array from the end
			const UInt8* first = packet.current();
			const UInt8* cur = first+packet.available()-1;
			while(cur>=first && isspace(*cur)) // ignore spaces at end
				--cur;

			// Delete </__array> from the end
			if(cur>=first && (cur-first) > 9 && *cur== '>') {
				cur -= 9;
				_tagName.resize(10);
				memcpy((UInt8*)_tagName.data(), cur, 10);
				if (_tagName == "</__array>")
		 			packet.shrink(cur-first);
			}
		}
	}
	_pos = packet.position();
	reset();
}

void XMLReader::reset() {
	packet.reset(_pos);
	_tagName.clear();
	_value.clear();
	_last=NIL;
	_dval=0;
	_tagOpened=false;
	_isProperty=false;
	_nextStep=NOTHING;
	_queueTags.clear();
	_nextType=NIL;
}

const UInt8* XMLReader::readBytes(UInt32& size) {
	_last=NIL;
	size = _value.size();
	return (const UInt8*)_value.c_str();
}

bool XMLReader::readBoolean() {
	_last=NIL;
	if(_last==STRING) {
		string tmp = _value;
		if (String::ToLower(tmp)=="true")
			return true;
		else if (tmp=="false")
			return false;
	}	

	return _dval > 0;
}

Date& XMLReader::readDate(Date& date) {
	_last=NIL;
	return date = _date;
}

string& XMLReader::readString(string& value) {
	_last=NIL;
	value.assign(_value);
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

	// First array
	if (_queueTags.empty()) {

		_queueTags.emplace_back("__array");
		if (_tagName != "__array") {
			TagXML& tag = _queueTags.back();
			tag.arrayStarted=false;
			_nextStep=START_ARRAY_TAG;
			tag.currentSubTag.assign(_tagName);
		}
	} 
	// Array
	else if (_tagName == "__array") {
		_queueTags.emplace_back("__array");
	}
	// Tag : start array of subtag
	else if (!_queueTags.empty()) {

		TagXML& tag = _queueTags.back();
		if (!tag.currentSubTag.empty())
			tag.arrayStarted=true;
	}

	return true;
}

XMLReader::Type XMLReader::readItem(string& name) {
	const UInt8* cur = current();
	if(!cur)
		return END;
	if (!name.empty())
		name.assign("");
	
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
				ERROR("XML malformed, tag ", _tagName," does not terminate")
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

	// New tag or Attribute : assign name before read value
	if (_isProperty) {
		_isProperty = false;
		if (_tagName != "__array")
			name.assign(_tagName);
	}

	return type;
}

DataReader::Type XMLReader::followingType() {
	if(_last!=NIL && _last!=END)
		return _last;

	const UInt8* cur = current();
	if(!cur)
		return END;

	_last=END;
	// Send recorded type
	if (_nextType) {

		_last=_nextType;
		_nextType=NIL;
	}
	// Terminate suspended action
	else if (_nextStep) {

		switch(_nextStep) {
		case POP_TAG:
			_nextStep=NOTHING;
			_last = removeTag();
			break;
		case ADD_TAG:
			_last = addTag(cur);
			break;
		case START_ARRAY_TAG:
			_last = addTag(cur, true);
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
			return parseComment(first);

		// Read Tag name
		do {
			packet.next();
		} while((cur=packet.current()) && *cur!='>' && (cur==first || *cur!='/') && !isspace(*cur));
		_tagName.resize(cur-first);
		memcpy((UInt8*)_tagName.data(), first, cur-first);

		// Got to next char
		cur=current();

		if (cur==NULL)
			ERROR("XML malformed, tag ", _tagName, " does not terminate")
		// Special case : Primitive with ignored tag
		else if (_tagName == "__noname" || _tagName == "/__noname") {
			packet.next();

			 // Try to end last subtag array, otherwise return following
			if (!endSubtagArray())
				_last = XMLReader::followingType();
		}
		// Start of an array
		else if (_tagName == "__array") {
			packet.next();
			_last=ARRAY; // readed in next readItem
			// Try to end last subtag array, next type will be ARRAY
			if (endSubtagArray())
				return END;
		}
		// End tag
		else if (*first == '/') {
			_tagName.erase(_tagName.begin()); // Erase '/'
			packet.next();

			if (_queueTags.empty()) {
				ERROR("End of tag ", _tagName, " without start")
				return END;
			}

			TagXML& tag = _queueTags.back();
			// End of child tag
			if (tag.tagName==_tagName && !tag.currentSubTag.empty()) {
				tag.currentSubTag.assign("");
				_nextStep=POP_TAG;
			} 
			// End of parent tag
			else 
				_last = removeTag();
		}
		// New subtag
		else if (!_queueTags.empty()) {

			TagXML& tag = _queueTags.back();
			// End of last Array
			if (tag.arrayStarted && tag.currentSubTag != _tagName) {
				tag.currentSubTag.assign(_tagName);
				tag.arrayStarted=false;
				_nextStep=START_ARRAY_TAG;
			} else {
				if (tag.currentSubTag.empty())
					tag.currentSubTag.assign(_tagName);

				_last = addTag(cur, !tag.arrayStarted);
			}
		}
		// New tag
		else
			_last = addTag(cur, true);
	}
	// Value
	else if(isalnum(*cur) || *cur=='-' || *cur=='\"') {
		// Try to end last subtag array, otherwise return primitive
		if (!endSubtagArray())
			_last = parsePrimitive(cur);
	}

	return _last;
}

XMLReader::Type XMLReader::parseComment(const UInt8* first) {

	bool finished = false;
	const UInt8* cur = NULL;
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

	_value.resize(cur-first);
	memcpy((UInt8*)_value.data(), first, cur-first);
	String::Trim(_value);

	Type type = STRING;
	Exception ex;
	_dval = String::ToNumber<double>(ex, _value);
	if (!ex) 
		type = NUMBER;
	// TODO do not return time if format is not a date ("the end" return a time)
	else if (_value.size() > 18 && _value.size() < 34) {
		_date.update(ex, _value);
		if (!ex)
			type = DATE;
	}

	if(chained)
		packet.next(1); // remove last '"'

	return type;
}

XMLReader::Type XMLReader::addTag(const UInt8* cur, bool evaluateArray) {		

	// Object with no attributes and no sub tag
	Type res = ARRAY;
	if (*cur=='>' && (isalnum(*(cur+1)) || *(cur+1)=='-' || *(cur+1)=='\"')) {

		_nextStep=NOTHING;
		packet.next();
		res = parsePrimitive(packet.current());

		cur=current();
		// Is tag well closed?
		if (packet.available() < _tagName.size() + 3) {
			ERROR("Tag '", _tagName, "' not well finished")
			return END;
		}
		string tmp;
		packet.readRaw(_tagName.size() + 3, tmp);
		string tmpTag;
		if (String::ICompare(String::Format(tmpTag, "</", _tagName, ">"), tmp) != 0) {
			ERROR("Tag '", _tagName, "' not well finished")
			return END;
		}
		
		// If array need to be created : record type and create array
		if (evaluateArray) {
			_isProperty=true;
			if (nextTagIsSame(_tagName)) {
				_nextType = res;
				res = ARRAY;
			} else
				endSubtagArray(); // erase subtag name
		}
	} else {

		if (evaluateArray) { // first create array
			_isProperty=true;
			_nextStep = ADD_TAG;
		} else { // then create tag
			_nextStep=NOTHING;
			_queueTags.emplace_back(_tagName);
			_tagOpened=true;
		}
	}

	return res;
}

bool XMLReader::nextTagIsSame(const string& name) {

	const UInt8* cur=current();
	if (packet.available() > name.size() + 3) {
		string tmp;
		tmp.resize(name.size()+3);
		memcpy((UInt8*)tmp.data(), cur, name.size() + 3);

		// Next tag has the same name : return ARRAY
		string tagOpen;
		if (String::ICompare(String::Format(tagOpen, '<', name, '>'), tmp, name.size() + 2) == 0)
			return true;

		if (String::ICompare(String::Format(tagOpen, '<', name, "/>"), tmp) == 0)
			return true;
	}

	return false;
}

XMLReader::Type XMLReader::removeTag() {
	
	_queueTags.pop_back();
	return END;
}

bool XMLReader::endSubtagArray() {

	if (!_queueTags.empty()) {
		TagXML& tag = _queueTags.back();
		if (!tag.currentSubTag.empty()) {
			tag.currentSubTag.assign("");
			return true;
		}
	}

	return false;
}

const UInt8* XMLReader::current() {
	while(available() && isspace(*packet.current()))
		packet.next(1);
	if(!available())
		return NULL;
	return packet.current();
}

} // namespace Mona
