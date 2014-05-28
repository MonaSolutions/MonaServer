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

#include "Script.h"
#include "Mona/Logs.h"
#include "Mona/Util.h"
#include <math.h>
extern "C" {
	#include "luajit-2.0/lualib.h"
}

using namespace std;
using namespace Mona;

lua_Debug	Script::LuaDebug;

const char* Script::LastError(lua_State *pState) {
	int top = lua_gettop(pState);
	if (top == 0)
		return "Unknown error";
	const char* error = lua_tostring(pState, -1);
	lua_pop(pState, 1);
	if (!error)
		return "Unknown error";
	return error;
}

int Script::Error(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		string msg;
		SCRIPT_LOG(Mona::Logger::LEVEL_ERROR,__FILE__,__LINE__, false, ToPrint(pState,msg))
	SCRIPT_END
	return 0;
}
int Script::Warn(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		string msg;
		SCRIPT_LOG(Mona::Logger::LEVEL_WARN,__FILE__,__LINE__, false, ToPrint(pState,msg))
	SCRIPT_END
	return 0;
}
int Script::Note(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		string msg;
		SCRIPT_LOG(Mona::Logger::LEVEL_NOTE,__FILE__,__LINE__, false, ToPrint(pState,msg))
	SCRIPT_END
	return 0;
}
int Script::Info(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		string msg;
		SCRIPT_LOG(Mona::Logger::LEVEL_INFO,__FILE__,__LINE__, false, ToPrint(pState,msg))
	SCRIPT_END
	return 0;
}
int Script::Debug(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		string msg;
		SCRIPT_LOG(Mona::Logger::LEVEL_DEBUG,__FILE__,__LINE__, false, ToPrint(pState,msg))
	SCRIPT_END
	return 0;
}
int Script::Trace(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		string msg;
		SCRIPT_LOG(Mona::Logger::LEVEL_TRACE,__FILE__,__LINE__, false, ToPrint(pState,msg))
	SCRIPT_END
	return 0;
}


int Script::Panic(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		string msg;
		SCRIPT_FATAL(ToPrint(pState,msg));
	SCRIPT_END
	return 1;
}

lua_State* Script::CreateState() {
	lua_State* pState = lua_open();
	luaL_openlibs(pState);
	lua_atpanic(pState,&Script::Panic);

	lua_pushcfunction(pState,&Script::Error);
	lua_setglobal(pState,"ERROR");
	lua_pushcfunction(pState,&Script::Warn);
	lua_setglobal(pState,"WARN");
	lua_pushcfunction(pState,&Script::Note);
	lua_setglobal(pState,"NOTE");
	lua_pushcfunction(pState,&Script::Info);
	lua_setglobal(pState,"INFO");
	lua_pushcfunction(pState,&Script::Debug);
	lua_setglobal(pState,"DEBUG");
	lua_pushcfunction(pState,&Script::Trace);
	lua_setglobal(pState,"TRACE");

	// set global metatable
	lua_newtable(pState);
#if !defined(_DEBUG)
	lua_pushstring(pState, "change metatable of global environment is prohibited");
	lua_setfield(pState, -2, "__metatable");
#endif
	lua_setmetatable(pState, LUA_GLOBALSINDEX);

	return pState;
}

void Script::CloseState(lua_State* pState) {
	if(pState)
		lua_close(pState);
}

void Script::PushValue(lua_State* pState,const char* value) {
	if (String::ICompare(value, "false")==0 || String::ICompare(value, "null") == 0)
		lua_pushboolean(pState, 0);
	else
		lua_pushstring(pState, value);
}

void Script::PushValue(lua_State* pState,const UInt8* value, UInt32 size) {
	if ((size==5 && String::ICompare((const char*)value, "false") == 0) || (size==4 && String::ICompare((const char*)value, "null") == 0))
		lua_pushboolean(pState, 0);
	else
		lua_pushlstring(pState, (const char*)value, size);
}

void Script::FillCollection(lua_State* pState, UInt32 size) {
	int index = -1-(size<<1); // index collection (-1-2*size)
	int count(0);
	while (size-- > 0) {
		
		if (lua_isstring(pState, -2)) { // key
			const char* key(lua_tostring(pState, -2));
			const char* sub(strchr(key, '.'));
			if (sub) {
				*(char*)sub = '\0';
				Collection(pState, index, key);
				*(char*)sub = '.';
				lua_pushstring(pState, sub + 1); // sub key
				lua_pushvalue(pState, -3); // value
				FillCollection(pState, 1);
				lua_pop(pState, 1);
			}
		}

		// check if key exists already to update count
		lua_pushvalue(pState, -2); // key
		lua_rawget(pState, index-1);
		if (!lua_isnil(pState, -1)) { // if old value exists
			if (lua_isnil(pState, -2)) // if new value is nil (erasing)
				--count;
		} else if (lua_isnil(pState, -1))  // if old value doesn't exists
			++count;
		lua_pop(pState, 1);

		lua_rawset(pState,index);
		index += 2;
	}
	
	// update count
	if (!lua_getmetatable(pState, -1)) {
		SCRIPT_BEGIN(pState)
			SCRIPT_WARN("Impossible to update count value of this collection, no metatable")
		SCRIPT_END
		return;
	}
	lua_getfield(pState,-1,"|count");
	lua_pushnumber(pState,lua_tonumber(pState,-1)+count);
	lua_replace(pState, -2);
	lua_setfield(pState,-2,"|count");
	lua_pop(pState,1); // remove metatable

}

void Script::ClearCollection(lua_State* pState) {
	// Clear content
	lua_pushnil(pState);  // first key 
	while (lua_next(pState, -2) != 0) {
		// uses 'key' (at index -2) and 'value' (at index -1) 
		// remove the raw!
		lua_pushvalue(pState, -2); // duplicate key
		lua_pushnil(pState);
		lua_rawset(pState, -5);
		lua_pop(pState, 1);
	}

	// update count
	if (!lua_getmetatable(pState, -1))
		return;
	lua_pushnumber(pState, 0);
	lua_setfield(pState, -2, "|count");
	lua_pop(pState, 1);
}

void Script::ClearCollectionParameters(lua_State* pState, const char* field,const Parameters& parameters) {
	// index -1 must be the collection
	if (!lua_getmetatable(pState, -1))
		return;
	string buffer;

	lua_getfield(pState, -1, String::Format(buffer,"|",field,"OnChange").c_str());
	Parameters::OnChange::Type* pOnChange((Parameters::OnChange::Type*)lua_touserdata(pState, -1));
	lua_pop(pState, 1);
	lua_pushnil(pState);
	lua_setfield(pState, -2, buffer.c_str());

	lua_getfield(pState, -1, String::Format(buffer,"|",field,"OnClear").c_str());
	Parameters::OnClear::Type* pOnClear((Parameters::OnClear::Type*)lua_touserdata(pState, -1));
	lua_pop(pState, 1);
	lua_pushnil(pState);
	lua_setfield(pState, -2, buffer.c_str());

	lua_pop(pState, 1); // metatable

	if (pOnChange) {
		parameters.OnChange::unsubscribe(*pOnChange);
		delete pOnChange;
	}
	if (pOnClear) {
		parameters.OnClear::unsubscribe(*pOnClear);
		delete pOnClear;
	}
}

int Script::Item(lua_State *pState) {
	SCRIPT_BEGIN(pState)
		SCRIPT_ERROR("This collection doesn't implement call operator, use [] operator rather")
	SCRIPT_END
	return 0;
}


int Script::IndexCollection(lua_State* pState) {
	// 1 table
	// 2 key
	const char* name(lua_tostring(pState, 2));
	if (name) {
		// search possible sub key
		if (lua_getmetatable(pState, 1)) {
			lua_getfield(pState, -1, name);
			lua_replace(pState, -2);
			if (!lua_isnil(pState, -1)) {
				lua_pushstring(pState, name);
				lua_pushvalue(pState, -2);
				lua_rawset(pState, 1);
				return 1;
			}
			lua_pop(pState, 1);
		}
		if (strcmp(name, "count") == 0) {
			lua_pushnumber(pState,lua_objlen(pState, 1));
			lua_pushstring(pState, name);
			lua_pushvalue(pState, -2);
			lua_rawset(pState, 1);
			return 1;
		}
	}
	return 0;
}

int Script::NewIndexCollection(lua_State* pState) {
	// 1 table
	// 2 key
	// 3 value
	SCRIPT_BEGIN(pState)
		SCRIPT_ERROR("This collection is read-only")
	SCRIPT_END
	return 0;
}

int Script::Len(lua_State* pState) {
	// 1 table
	if(lua_getmetatable(pState, 1)) {
		lua_getfield(pState, -1, "|count");
		if (lua_isnumber(pState, -1)) {
			lua_replace(pState, -2);
			return 1;
		}
		lua_pop(pState, 2);
	}
	lua_pushnumber(pState,lua_objlen(pState, 1));
	return 1;
}


DataReader& Script::WriteData(lua_State *pState,DataReader& reader,UInt32 count) {
	DataReader::Type type;
	bool all=count==0;
	if(all)
		count=1;
	while(count>0 && (type = reader.followingType())!=DataReader::END) {
		WriteData(pState,type,reader);
		if(!all)
			--count;
	}
	return reader;
}


DataReader& Script::WriteData(lua_State *pState,DataReader::Type type,DataReader& reader) {
	SCRIPT_BEGIN(pState)
	switch(type) {
		case DataReader::NIL:
			reader.readNull();
			lua_pushnil(pState);
			break;
		case DataReader::BOOLEAN:
			lua_pushboolean(pState,reader.readBoolean());
			break;
		case DataReader::NUMBER:
			lua_pushnumber(pState,reader.readNumber());
			break;
		case DataReader::STRING: {
			string value;
			reader.readString(value);
			lua_pushlstring(pState,value.c_str(),value.size());
			break;
		}
		case DataReader::DATE: {
			Date date;
			reader.readDate(date);
			lua_newtable(pState);
			lua_pushnumber(pState, (double)date);
			lua_setfield(pState, -2, "__time");
			lua_pushnumber(pState, date.year());
			lua_setfield(pState, -2, "year");
			lua_pushnumber(pState, date.month() + 1);
			lua_setfield(pState, -2, "month");
			lua_pushnumber(pState, date.day());
			lua_setfield(pState, -2, "day");
			lua_pushnumber(pState, date.yearDay());
			lua_setfield(pState, -2, "yday");
			lua_pushnumber(pState, date.weekDay());
			lua_setfield(pState, -2, "wday");
			lua_pushnumber(pState, date.hour());
			lua_setfield(pState, -2, "hour");
			lua_pushnumber(pState, date.minute());
			lua_setfield(pState, -2, "min");
			lua_pushnumber(pState, date.second());
			lua_setfield(pState, -2, "sec");
			lua_pushnumber(pState, date.millisecond());
			lua_setfield(pState, -2, "msec");
			lua_pushboolean(pState, date.isDST() ? 1 : 0);
			lua_setfield(pState, -2, "isdst");
			break;
		}
		case DataReader::ARRAY: {
			UInt32 size=0;
			if(reader.readArray(size)) {
				lua_newtable(pState);
				UInt32 index=0;
				string name;
				while((type=reader.readItem(name))!=DataReader::END) {
					WriteData(pState,type,reader);
					if(name.empty())
						lua_rawseti(pState,-2,++index);
					else
						lua_setfield(pState,-2,name.c_str());
				}
			}
			break;
		}
		case DataReader::MAP: {
			bool weakKeys=false;
			UInt32 size;
			if(reader.readMap(size,weakKeys)) {
				lua_newtable(pState);
				if(weakKeys) {
					lua_newtable(pState);
					lua_pushstring(pState,"k");
					lua_setfield(pState,-2,"__mode");
					lua_setmetatable(pState,-2);
				}
				string name;
				while((type=reader.readKey())!=DataReader::END) {
					WriteData(pState,type,reader);
					WriteData(pState,reader.readValue(),reader);
					lua_rawset(pState,-3);
				}
				lua_pushnumber(pState,size);
				lua_setfield(pState,-2,"__size");
			}
			break;
		}
		case DataReader::OBJECT: {
			string objectType;
			bool external=false;
			if(reader.readObject(objectType,external)) {
				lua_newtable(pState);
				if(!objectType.empty()) {
					lua_pushlstring(pState,objectType.c_str(),objectType.size());
					lua_setfield(pState,-2,"__type");
				} else if(external) {
					SCRIPT_ERROR("Impossible to deserialize the external object without a type")
					break;
				}
				string name;
				while((type=reader.readItem(name))!=DataReader::END) {
					WriteData(pState,type,reader);
					lua_setfield(pState,-2,name.c_str());
				}
				lua_getfield(pState,-1,"__type");
				if(lua_isstring(pState,-1)==1)
					objectType = lua_tostring(pState,-1);
				lua_pop(pState,1);
				if(!objectType.empty()) {
					int top = lua_gettop(pState);
					// function
					SCRIPT_FUNCTION_BEGIN("onTypedObject")
						// type argument
						lua_pushlstring(pState,objectType.c_str(),objectType.size());
						// table argument
						lua_pushvalue(pState,top);
						SCRIPT_FUNCTION_CALL
					SCRIPT_FUNCTION_END
				}
				// After the "onTypedObject" to get before the "__readExternal" required to unserialize
				lua_getfield(pState,-1,"__readExternal");
				if(lua_isfunction(pState,-1)) {
					// self
					lua_pushvalue(pState,-2);
					// reader
					SCRIPT_WRITE_BINARY(reader.packet.current(),reader.packet.available())
					if(lua_pcall(pState,2,1,0)!=0)
						SCRIPT_ERROR(Script::LastError(pState))
					else {
						reader.packet.next((int)lua_tonumber(pState,-1));
						lua_pop(pState,1);
					}
					break;
				} else if(external)
					SCRIPT_ERROR("Impossible to deserialize the external type '",objectType,"' without a __readExternal method")
				lua_pop(pState,1);
			}
			break;
		}
		case DataReader::BYTES: {
			UInt32 size;
			lua_newtable(pState);
			const UInt8* bytes = reader.readBytes(size);
			lua_pushlstring(pState,(const char*)bytes,size);
			lua_setfield(pState,-2,"__raw");
			break;
		}
		default:
			reader.packet.next(1);
			SCRIPT_ERROR("AMF ",type," type unknown");
			break;
	}
	SCRIPT_END
	return reader;
}

DataWriter& Script::ReadData(lua_State* pState,DataWriter& writer,UInt32 count) {
	map<UInt64,UInt32> references;
	return ReadData(pState,writer,count,references);
}

DataWriter& Script::ReadData(lua_State* pState,DataWriter& writer,UInt32 count,map<UInt64,UInt32>& references) {
	SCRIPT_BEGIN(pState)
	int top = lua_gettop(pState);
	Int32 args = top-count;
	if(args<0) {
		SCRIPT_ERROR("Impossible to write ",-args," missing AMF arguments")
		args=0;
	}
	while(args++<top) {
		int type = lua_type(pState, args);
		switch (type) {
			case LUA_TTABLE: {

				// Repeat
				UInt64 reference = (UInt64)lua_topointer(pState,args);
				map<UInt64,UInt32>::iterator it = references.lower_bound(reference);
				if(it!=references.end() && it->first==reference) {
					if(writer.repeat(it->second))
						break;
				} else { 
					if(it!=references.begin())
						--it;
					it = references.insert( it,pair<UInt64,UInt32>(reference,0));
				}

				// ByteArray
				lua_getfield(pState,args,"__raw");
				if(lua_isstring(pState,-1)) {
					UInt32 size = lua_objlen(pState,-1);
					writer.writeBytes((const UInt8*)lua_tostring(pState,-1),size);
					it->second = writer.lastReference();
					lua_pop(pState,1);
					break;
				}
				lua_pop(pState,1);

				// Date
				lua_getfield(pState,args,"__time");
				if(lua_isnumber(pState,-1)) {
					Date date((Int64)lua_tonumber(pState,-1));
					writer.writeDate(date);
					it->second = writer.lastReference();
					lua_pop(pState,1);
					break;
				}
				lua_pop(pState,1);


				// Dictionary
				UInt32 size=0;
				lua_getfield(pState,args,"__size");
				if(lua_isnumber(pState,-1)) {
					size = (UInt32)lua_tonumber(pState,-1);
	
					bool weakKeys = false;
					if(lua_getmetatable(pState,args)!=0) {
						lua_getfield(pState,-1,"__mode");
						if(lua_isstring(pState,-1) && strcmp(lua_tostring(pState,-1),"k")==0)
							weakKeys=true;
						lua_pop(pState,2);
					}
					
					// Array, write properties in first
					writer.beginMap(size,weakKeys);
					lua_pushnil(pState);  /* first key */
					while (lua_next(pState, args) != 0) {
						/* uses 'key' (at index -2) and 'value' (at index -1) */
						if(lua_type(pState,-2)!=LUA_TSTRING || strcmp(lua_tostring(pState,-2),"__size")!=0 )
							ReadData(pState,writer,2);
						/* removes 'value'; keeps 'key' for next iteration */
						lua_pop(pState, 1);
					}
					writer.endMap();

					it->second = writer.lastReference();
					lua_pop(pState,1);
					break;
				}
				lua_pop(pState,1);


				bool object=true;
				bool start=false;
				if(size==0)
					size = luaL_getn(pState,args);
				
				// Object or Array!
				lua_getfield(pState,args,"__type");
				const char* objectType = lua_isstring(pState,-1) ? lua_tostring(pState,-1) : NULL;
				if(objectType) {
					// Externalized?
					lua_getfield(pState,args,"__writeExternal");
					if(lua_isfunction(pState,-1)) {
						writer.beginObject(objectType,true);
						// self argument
						lua_pushvalue(pState,args);
						if(lua_pcall(pState,2,1,0)!=0)
							SCRIPT_ERROR(Script::LastError(pState))
						else {
							writer.packet.writeRaw((const UInt8*)lua_tostring(pState,-1),lua_objlen(pState,-1));
							lua_pop(pState,1);
						}
						writer.endObject();
						it->second = writer.lastReference();
						lua_pop(pState,1);
						break;
					}
					lua_pop(pState,1);
					writer.beginObject(objectType);
					start=true;
				} else if(size>0) {
					// Array, write properties in first
					object=false;
					lua_pushnil(pState);  /* first key */
					while (lua_next(pState, args) != 0) {
						/* uses 'key' (at index -2) and 'value' (at index -1) */
						int keyType = lua_type(pState,-2);
						const char* key = NULL;
						if(keyType==LUA_TSTRING && strcmp((key=lua_tostring(pState,-2)),"__type")!=0 ) {
							if (!start) {
								writer.beginObjectArray(size);
								start=true;
							}
							writer.writePropertyName(key);
							ReadData(pState,writer,1);
						}
						/* removes 'value'; keeps 'key' for next iteration */
						lua_pop(pState, 1);
					}
					if (!start) {
						writer.beginArray(size);
						start=true;
					} else
						writer.endObject();
				}
				lua_pop(pState,1);
				

				lua_pushnil(pState);  /* first key */
				while (lua_next(pState, args) != 0) {
					/* uses 'key' (at index -2) and 'value' (at index -1) */
					int keyType = lua_type(pState,-2);
					const char* key = NULL;
					if(keyType==LUA_TSTRING) {
						if(object && strcmp((key=lua_tostring(pState,-2)),"__type")!=0) {
							if(!start) {
								writer.beginObject();
								start=true;
							}
							writer.writePropertyName(key);
							ReadData(pState,writer,1);
						}
					} else if(keyType==LUA_TNUMBER) {
						if(object)
							SCRIPT_WARN("Impossible to encode this array element in an AMF object format")
						else
							ReadData(pState,writer,1);
					} else
						SCRIPT_WARN("Impossible to encode this table key of type ",lua_typename(pState, keyType), " in an AMF object format")
					/* removes 'value'; keeps 'key' for next iteration */
					lua_pop(pState, 1);
				}
				if(start) {
					if(object)
						writer.endObject();
					else
						writer.endArray();
				} else {
					writer.beginArray(0);
					writer.endArray();
				}

				it->second = writer.lastReference();
				break;
			}
			case LUA_TSTRING: {
				string data(lua_tostring(pState,args),lua_objlen(pState,args));
				writer.writeString(data);
				break;
			}
			case LUA_TBOOLEAN:
				writer.writeBoolean(lua_toboolean(pState,args)==0 ? false : true);
				break;
			case LUA_TNUMBER: {
				writer.writeNumber(lua_tonumber(pState,args));
				break;
			}
			default:
				SCRIPT_WARN("Impossible to encode the '",lua_typename(pState,type),"' type in a AMF format")
			case LUA_TNIL:
				writer.writeNull();
				break;
		}
	}
	SCRIPT_END
	return writer;
}

const char* Script::ToPrint(lua_State* pState, string& out) {
	int top = lua_gettop(pState);
	int args = 0;
	while(args++<top) {
		string pointer;
		switch (lua_type(pState, args)) {
			case LUA_TLIGHTUSERDATA:
				pointer = "userdata_";
				break;
			case LUA_TFUNCTION:
				pointer = "function_";
				break;
			case LUA_TUSERDATA:
				pointer = "userdata_";
				break;
			case LUA_TTHREAD:
				pointer = "thread_";
				break;
			case LUA_TTABLE: {
				pointer = "table_";
				break;
			}
			case LUA_TBOOLEAN: {
				out += lua_toboolean(pState,args) ? "(true)" : "(false)";
				continue;
			}
			case LUA_TNIL: {
				out += "(null)";
				continue;
			}
		}
		if(pointer.empty())
			out += lua_tostring(pState,args);
		else
			String::Append(out, pointer, lua_topointer(pState, args));
	}
	return out.c_str();
}


void Script::Test(lua_State* pState) {
	int i;
	int top = lua_gettop(pState);

	printf("total in stack %d\n", top);

	for (i = 1; i <= top; i++) {  /* repeat for each level */
		int t = lua_type(pState, i);
		switch (t) {

		case LUA_TNONE:  /* strings */
			printf("none\n");
			break;
		case LUA_TNIL:  /* strings */
			printf("nil\n");
			break;
		case LUA_TSTRING:  /* strings */
			printf("string: '%s'\n", lua_tostring(pState, i));
			break;
		case LUA_TBOOLEAN:  /* booleans */
			printf("boolean %s\n", lua_toboolean(pState, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:  /* numbers */
			printf("number: %g\n", lua_tonumber(pState, i));
			break;
		default:  /* other values */
			printf("%s: %p\n", lua_typename(pState, t),lua_topointer(pState,i));
			break;
		}
		printf("  ");  /* put a separator */
	}
	printf("\n");  /* end the listing */

}

