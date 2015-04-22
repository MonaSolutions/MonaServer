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

#include "ScriptReader.h"
#include "ScriptWriter.h"

using namespace std;
using namespace Mona;

ScriptReader::ScriptReader(lua_State *pState, UInt32 count) : _pState(pState), _start(lua_gettop(pState)) {
	if ((int)count > _start)
		count = _start;
	++_start;
	_start -= count;
	_current = _start;
	_end = _current + count;
}

#if defined(_DEBUG)
UInt32 ScriptReader::read(DataWriter& writer, UInt32 count) {
	// DEBUG causes this method is used a lot and dynamic_cast is cpu expensive
	if (dynamic_cast<ScriptWriter*>(&writer))
		CRITIC("A ScriptReader is writing to a ScriptWriter, behavior undefined (unsafe)")
	return ReferableReader::read(writer, count);
}
#endif


void ScriptReader::reset() {
	_current = _start;
	DataReader::reset();
}

bool ScriptReader::writeNext(DataWriter& writer) {
	int current(_current);
	_current = lua_gettop(_pState);
	bool result(readNext(writer));
	_current = current;
	return result;
}

UInt8 ScriptReader::followingType() {
	if (_current == _end)
		return END;

	switch (lua_type(_pState, _current)) {
		case LUA_TSTRING:
			return STRING;
		case LUA_TBOOLEAN:
			return BOOLEAN;
		case LUA_TNUMBER:
			return NUMBER;
		case LUA_TNIL:
			return NIL;
		case LUA_TTABLE: {
			lua_getfield(_pState, _current, "__raw");
			if (lua_isstring(_pState, -1)) {
				lua_pop(_pState, 1);
				return BYTES;
			}
			lua_pop(_pState, 1);

			lua_getfield(_pState, _current, "__time");
			if (lua_isnumber(_pState, -1)) {
				lua_pop(_pState, 1);
				return DATE;
			}
			lua_pop(_pState, 1);
		}
	}
	return OTHER;
}

bool ScriptReader::readOne(UInt8 type,DataWriter& writer) {
	
	SCRIPT_BEGIN(_pState)

	UInt32 reference((Mona::UInt64)lua_topointer(_pState, _current));
	if (reference && tryToRepeat(writer, reference)) {
		++_current;
		return true;
	}

	switch (type) {

		case STRING:  {
			writer.writeString(lua_tostring(_pState, _current), lua_objlen(_pState, _current));
			++_current;
			return true;
		}
		case BOOLEAN:
			writer.writeBoolean(lua_toboolean(_pState,_current++)==0 ? false : true);
			return true;
		case NUMBER:
			writer.writeNumber(lua_tonumber(_pState,_current++));
			return true;
		case NIL:
			writer.writeNull();
			++_current;
			return true;

		case BYTES:
			lua_getfield(_pState, _current++, "__raw");
			writeBytes(writer,reference,(const UInt8*)lua_tostring(_pState,-1),lua_objlen(_pState,-1));
			lua_pop(_pState,1);
			return true;

		case DATE: {
			lua_getfield(_pState, _current++, "__time");
			Date date((Int64)lua_tonumber(_pState, -1));
			writeDate(writer,reference,date);
			lua_pop(_pState,1);
			return true;
		}

	}

	bool object=true;
	bool started=false;
	Reference* pReference(NULL);
	UInt32 size(0);

	// Dictionary
	lua_getfield(_pState,_current,"__size");
	if(lua_isnumber(_pState,-1)) {
		size = (UInt32)lua_tonumber(_pState,-1);
	
		bool weakKeys = false;
		if(lua_getmetatable(_pState,_current)!=0) {
			lua_getfield(_pState,-1,"__mode");
			if(lua_isstring(_pState,-1) && strcmp(lua_tostring(_pState,-1),"k")==0)
				weakKeys=true;
			lua_pop(_pState,2);
		}

		Exception ex;
		pReference = beginMap(writer,reference,ex,size,weakKeys);
		if (ex) {
			SCRIPT_WARN(ex.error());
			started = true;
		} else {
			lua_pushnil(_pState);  /* first key */
			while (Script::Next(_pState, _current) != 0) {
				/* uses 'key' (at index -2) and 'value' (at index -1) */
				if (lua_type(_pState, -2) != LUA_TSTRING || strcmp(lua_tostring(_pState, -2), "__size") != 0) {
					 // key
					lua_pushvalue(_pState, -2);
					if (!writeNext(writer))
						writer.writeNull();
					lua_pop(_pState, 1);
					// value
					if (!writeNext(writer))
						writer.writeNull();
				}
				/* removes 'value'; keeps 'key' for next iteration */
				lua_pop(_pState, 1);
			}
			endMap(writer,pReference);

			lua_pop(_pState,1);
			++_current;
			return true;
		}
	}
	lua_pop(_pState,1);

	
	// Object or Array!
	if (!started) {
		lua_getfield(_pState,_current,"__type");
		if(lua_isstring(_pState,-1)) {
			pReference = beginObject(writer,reference,lua_tostring(_pState,-1));
			started=true;
		} else if((size = lua_objlen(_pState,_current))) {
			// Array or mixed, write properties in first
			object=false;
			lua_pushnil(_pState);  /* first key */
			while (Script::Next(_pState, _current) != 0) {
				/* uses 'key' (at index -2) and 'value' (at index -1) */
				if(lua_type(_pState,-2)==LUA_TSTRING) {
					if (!started) {
						pReference = beginObjectArray(writer,reference,size);
						started=true;
					}
					writer.writePropertyName(lua_tostring(_pState,-2));
					if (!writeNext(writer))
						writer.writeNull();
				}
				/* removes 'value'; keeps 'key' for next iteration */
				lua_pop(_pState, 1);
			}
			if (!started) {
				pReference = beginArray(writer,reference,size);
				started=true;
			} else
				endObject(writer,pReference);
		}
		lua_pop(_pState,1);
	}
				

	lua_pushnil(_pState);  /* first key */
	while (Script::Next(_pState, _current) != 0) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		if (lua_type(_pState,-2)==LUA_TSTRING) {
			// here necessarly we are writing an object (mixed or pure object)
			if (object) { // if pure object
				const char* key(lua_tostring(_pState, -2));
				if (strcmp(key, "__type") != 0) {
					if (!started) {
						pReference = beginObject(writer,reference);
						started = true;
					}
					writer.writePropertyName(key);
					if (!writeNext(writer))
						writer.writeNull();
				}
			} // else already written!
		} else if (lua_type(_pState, -2) == LUA_TNUMBER) { // string have already been written for an array
			if (object) { // pure object!
				string prop;
				writer.writePropertyName(String::Format(prop, lua_tonumber(_pState, -2)).c_str());
				if (!writeNext(writer))
					writer.writeNull();
			} else
				writeNext(writer);
		} else
			SCRIPT_WARN("Impossible to encode the key of type ",lua_typename(_pState, lua_type(_pState,-2)), " in an acceptable object format")
		/* removes 'value'; keeps 'key' for next iteration */
		lua_pop(_pState, 1);
	}
	if(started) {
		if(object)
			endObject(writer,pReference);
		else
			endArray(writer,pReference);
	} else {
		pReference = beginArray(writer,reference,0);
		endArray(writer,pReference);
	}

	++_current;

	SCRIPT_END

	return true;

}
