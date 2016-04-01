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

#include "Mona/XMLParser.h"

using namespace std;

namespace Mona {

void XMLParser::reset() {
	_reseted = true;
	_started=false;
	_ex.set(Exception::NIL);
	_current = _start;
	_tags.clear();
}

void XMLParser::reset(const XMLState& state) {
	if (!state)
		return reset();
	_reseted = true;
	_started = state._started;
	_ex = state._ex;
	_current = state._current;
	_tags = state._tags;
}

void XMLParser::save(XMLState& state) {
	state._started = _started;
	state._current = _current;
	state._tags    = _tags;
	state._ex = _ex;
}

bool XMLParser::parse(Exception& ex) {
	if (_running) {
		ex.set(Exception::SOFTWARE, "Impossible to call recursively XMLParser::parse function");
		return false;
	}
	_running = true;
	bool rest(false);
	if (_ex)
		ex = _ex;
	else {
		rest = parse();
		if (!rest)
			onEndXMLDocument(_ex ? (ex=_ex).error() : NULL);
	}
	_running = false;
	return rest;
}

bool XMLParser::parse() {

RESET:
	_reseted = false;
	if (_ex)
		return false;

	UInt32 size(0);
	char*  name(NULL);

	while (!_tags.empty()) {
		Tag& tag(_tags.back());
		if (!tag.full)
			break;
		bool next(false);
		SCOPED_STRINGIFY( name = tag.name, size = tag.size, _tags.pop_back(); next = onEndXMLElement(name);) 
		if (!next)
			return (_current == _end && _tags.empty()) ? false : true;
		if (_reseted)
			goto RESET;
	}

	bool isInner(false);
	UInt32 innerSpaceRemovables(0);

	while (_current<_end) {

		// start element or end element
		if (*_current == '<') {

			if (!_started) {
				_started = true; // before to allow a call to save
				if (!onStartXMLDocument())
					return true;
				if (_reseted)
					goto RESET;
			}

			// process inner!
			if (isInner && !((_end - _current) >= 9 && memcmp(_current, "<![CDATA[", 9) == 0)) {
	
				// skip end space if not CDATA
				_inner.clear(_inner.size() - innerSpaceRemovables);
				_inner.write8(0); // string null termination
		
				bool next(false);
				Tag& tag(_tags.back());
				SCOPED_STRINGIFY( tag.name, tag.size, next = onInnerXMLElement(tag.name, STR _inner.data(), _inner.size()-1);) 

				isInner = false;
				innerSpaceRemovables = 0;

				if (!next)
					return true;
				if (_reseted)
					goto RESET;

			}

			if (++_current == _end) {
				_ex.set(Exception::FORMATTING, "XML element without tag name");
				return false;
			}

			// just after <

			bool next(true);
				
			if (*_current == '/') {
				//// END ELEMENT ////

				if (_tags.empty()) {
					_ex.set(Exception::FORMATTING, "XML end element without starting before");
					return false;
				}

				Tag& tag(_tags.back());
				name = tag.name;
				size = tag.size;

				if ((_current+size) >= _end || memcmp(name,++_current,size)!=0) {
					_ex.set(Exception::FORMATTING, "XML end element is not the '",std::string(name,size),"' expected");
					return false;
				}
				_current += size;

				// skip space
				while (_current < _end && isspace(*_current))
					++_current;
			
				if (_current == _end || *_current!='>') {
					_ex.set(Exception::FORMATTING, "XML end '",std::string(name,size),"' element without > termination");
					return false;
				}
	
				// on '>'

				// skip space
				++_current;
				if (!isInner) while (_current < _end && isspace(*_current)) ++_current;

				SCOPED_STRINGIFY(name, size, _tags.pop_back(); next = onEndXMLElement(name);) 


			} else if (*_current == '!') {
				//// COMMENT or CDATA ////

				if (++_current == _end) {
					_ex.set(Exception::FORMATTING, "XML comment or CDATA malformed");
					return false;
				}

				if (isInner || ((_end - _current) >= 7 && memcmp(_current, "[CDATA[", 7) == 0)) {
					/// CDATA ///

					if (_tags.empty()) {
						_ex.set(Exception::FORMATTING, "No XML CDATA inner value possible without a XML parent element");
						return false;
					}

					_current += 7;
					// can be end

					if (!isInner) {
						isInner = true;
						_inner.clear();
					} else
						innerSpaceRemovables = 0;
					while (_current < _end) {
						if (*_current == ']' && ++_current < _end && *_current == ']' && ++_current < _end && *_current == '>')
							break;
						_inner.write8(*_current++);
					}
					
					if (_current == _end) {
						_ex.set(Exception::FORMATTING, "XML CDATA without ]]> termination");
						return false;
					}
				
					// on '>'

				} else {
					/// COMMENT ///

					char delimiter1 = *_current;
					if (++_current == _end) {
						_ex.set(Exception::FORMATTING, "XML comment malformed");
						return false;
					}
					char delimiter2 = *_current;

					while (++_current < _end) {
						if (*_current == delimiter1 && ++_current < _end && *_current == delimiter2 && ++_current < _end && *_current == '>')
							break;
					}

					if (_current == _end) {
						_ex.set(Exception::FORMATTING, "XML comment without > termination");
						return false;
					}

					// on '>'
				}
				
				// skip space
				++_current;
				if (!isInner) while (_current < _end && isspace(*_current)) ++_current;

			} else {

				//// START ELEMENT ////

				bool isInfos(*_current == '?');
				if (isInfos && ++_current == _end) {
					_ex.set(Exception::FORMATTING, "XML info element without name");
					return false;
				}

				Tag tag;
				name = tag.name = (char*)parseXMLName(isInfos ? "?" : "/>", tag.size);
				if (!name)
					return false;
				size = tag.size;
		
				/// space or > or /

				/// read attributes
				_attributes.clear();
				while (_current < _end) {

					// skip space
					while (isspace(*_current)) {
							if (++_current==_end)  {
							_ex.set(Exception::FORMATTING, "XML element '",string(name,size),"' without termination");
							return false;
						}
					}
				
					if (isInfos && (*_current == '?')) {
						if (++_current == _end || *_current!='>') {
							_ex.set(Exception::FORMATTING, "XML info '",string(name,size),"' without ?> termination");
							return false;
						}
						break;
					} else if (*_current == '/') {
						if (++_current == _end || *_current!='>') {
							_ex.set(Exception::FORMATTING, "XML element '",string(name,size),"' without termination");
							return false;
						}
						tag.full = true;
						break;
					} else if (*_current == '>')
						break;
	
					// read name attribute
					UInt32 sizeKey(0);
					const char* key(parseXMLName("=", sizeKey));
					if (!key)
						return false;

					/// space or =

					// skip space
					while(isspace(*_current)) {
						if (++_current == _end) {
							_ex.set(Exception::FORMATTING, "XML attribute '",string(key,sizeKey),"' without value");
							return false;
						}
						if (*_current == '=')
							break;
					}

					// =

					if (++_current==_end) {
						_ex.set(Exception::FORMATTING, "XML attribute '",string(key,sizeKey),"' without value");
						return false;
					}

					// skip space
					while(isspace(*_current)) {
						if (++_current == _end) {
							_ex.set(Exception::FORMATTING, "XML attribute '",string(key,sizeKey),"' without value");
							return false;
						}
					}

					/// just after = and space

					char delimiter(*_current);

					if ((delimiter != '"' && delimiter != '\'') || ++_current==_end) {
						_ex.set(Exception::FORMATTING, "XML attribute '",string(key,sizeKey),"' without value");
						return false;
					}

			

					/// just after " start of attribute value

					// read value attribute
					UInt32 sizeValue(0);
					const char* value(_current);
					while (*_current != delimiter) {
						if (*_current== '\\') {
							if (++_current == _end) {
								_ex.set(Exception::FORMATTING, "XML attribute '",string(key,sizeKey),"' with malformed value");
								return false;
							}
							++sizeValue;
						}
						if(++_current==_end) {
							_ex.set(Exception::FORMATTING, "XML attribute '",string(key,sizeKey),"' with malformed value");
							return false;
						}
						++sizeValue;
					}
						
					++_current;

					/// just after " end of attribute value

					// store name=value
					SCOPED_STRINGIFY(key, sizeKey, _attributes.setString(key,value,sizeValue) )
				}



				if (_current==_end)  {
					_ex.set(Exception::FORMATTING, "XML element '",string(name,size),"' without > termination");
					return false;
				}
			
				// on '>' ('/>' or  '?>' or '>')

				// skip space
				++_current;
				if (!isInner) while (_current < _end && isspace(*_current)) ++_current;

				SCOPED_STRINGIFY( name, size,
					if (isInfos)
						next = onXMLInfos(name, (Parameters&)_attributes);
					else {
						_tags.emplace_back(tag); // before to allow a call to save
						next = onStartXMLElement(name, (Parameters&)_attributes);
						if (next && tag.full) {
							_tags.pop_back();  // before to allow a call to save
							next = onEndXMLElement(name);
						}
					}
				) 
			}


			if (!next)
				return (_current == _end && _tags.empty()) ? false : true;
			if (_reseted)
				goto RESET;
	
			// after '>' (element end or element start) and after space, and can be _end

		} else {
			if (isInner) { // inner value
				_inner.write8(*_current);
				if (isspace(*_current))
					++innerSpaceRemovables;
				else
					innerSpaceRemovables = 0;
			} else if (!isspace(*_current)) {
				if (_tags.empty()) {
					_ex.set(Exception::FORMATTING, "No XML inner value possible without a XML parent element");
					break;
				}
				isInner = true;
				_inner.clear();
				_inner.write8(*_current);
			}	
			++_current;
		}
			
	}

	if (!_started) {
		_started = true;  // before to allow a call to save
		if (!onStartXMLDocument())
			return true;
		if (_reseted)
			goto RESET;
	} else if (!_tags.empty())
		_ex.set(Exception::FORMATTING, "No XML end '",std::string(_tags.back().name,_tags.back().size),"' element");

	return false; // no more data!
}


const char* XMLParser::parseXMLName(const char* endMarkers, UInt32& size) {

	if (!isalpha(*_current) && *_current!='_')  {
		_ex.set(Exception::FORMATTING, "XML name must start with an alphabetic character");
		return NULL;
	}
	const char* name(_current++);
	while (_current < _end && isxml(*_current))
		++_current;
	size = (_current - name);
	if (_current==_end || (!strchr(endMarkers,*_current) && !isspace(*_current)))  {
		_ex.set(Exception::FORMATTING, "XML name '",string(name,size),"' without termination");
		return NULL;
	}
	return name;
}

} // namespace Mona
