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

#include "Service.h"
#include "Mona/String.h"
#include "Mona/Logs.h"

#undef  SCRIPT_FIX_RESULT
#define SCRIPT_FIX_RESULT  { lua_pushvalue(pState,2); lua_pushvalue(pState, -2); lua_rawset(pState, 1);}


using namespace std;
using namespace Mona;

class Clients {
public:
	static int Item(lua_State *pState) {
		SCRIPT_CALLBACK(Service,service)	

			if (lua_isstring(pState, 2)) {

				SCRIPT_READ_BINARY(id, size)
		
				Script::Collection<Service>(pState, 1, "clients");
				
				UInt8 buffer[64];
				if ((id=Script::ToId(id, size, buffer))) {
					lua_pushlstring(pState,(const char*) id, size);
					lua_gettable(pState, -2);
				} else
					lua_pushnil(pState);

				lua_replace(pState, -2); // remove clients collection
			}

		SCRIPT_CALLBACK_RETURN
	}
};

Service::Service(lua_State* pState, const string& rootPath, ServiceHandler& handler) : _rootPath(rootPath), _lastCheck(0), _reference(LUA_REFNIL), _pParent(NULL), _handler(handler), _pState(pState), FileWatcher(rootPath, "/main.lua") {

}

Service::Service(lua_State* pState, const string& rootPath, Service& parent, const string& name, ServiceHandler& handler) : _rootPath(rootPath), name(name), _lastCheck(0), _reference(LUA_REFNIL), _pParent(&parent), _handler(handler), _pState(pState), FileWatcher(rootPath,parent.path,'/',name,"/main.lua") {
	String::Format((string&)path,parent.path,'/',name);
}

Service::~Service() {
	// clean children
	for (auto& it : _services)
		delete it.second;
	// clean this
	close(true);
}

void Service::setReference(int reference) {
	if (reference == _reference)
		return;
	// make obsolete the connected clients
	if (_reference != LUA_REFNIL) {
		lua_rawgeti(_pState, LUA_REGISTRYINDEX, _reference);
		Script::Collection(_pState, -1, "clients");
		lua_pushnil(_pState);  // first key 
		while (lua_next(_pState, -2) != 0) {
			bool isConst;
			// uses 'key' (at index -2) and 'value' (at index -1)
			Client* pClient = Script::ToObject<Client>(_pState, isConst);
			if (pClient && pClient->hasCustomData())
				*pClient->getCustomData<int>() = LUA_REFNIL;
			lua_pop(_pState, 2);
		}
		lua_pop(_pState, 2);
		luaL_unref(_pState, LUA_REGISTRYINDEX, _reference);
	}
	_reference = reference;
}

Service* Service::open(Exception& ex) {
	if (_lastCheck.isElapsed(2000)) { // already checked there is less of 2 sec!
		_lastCheck.update();
		if (!watchFile() && !path.empty() && !FileSystem::Exists(file.parent())) // no path/main.lua file, no main service, no path folder
			_ex.set(Exception::APPLICATION, "Application ", path, " doesn't exist");
	}
	
	if (_ex) {
		ex = _ex;
		return NULL;
	}

	// here => exists and no error on load
	open(true);
	return this;
}

Service* Service::open(Exception& ex, const string& path) {
	// remove first '/'
	string name;
	if(!path.empty())
		name.assign(path[0] == '/' ? &path.c_str()[1] : path.c_str());

	// substr first "service"
	size_t pos = name.find('/');
	string nextPath;
	if (pos != string::npos) {
		nextPath = &name.c_str()[pos];
		name.resize(pos);
	}

	Service* pSubService(this);
	auto it = _services.end();
	if (!name.empty()) {
		it = _services.lower_bound(name);
		if (it == _services.end() || it->first != name)
			it = _services.emplace_hint(it, name, new Service(_pState, _rootPath, *this, name, _handler)); // Service doesn't exists
		pSubService = it->second;
	}

	// if file or folder exists, return the service (or sub service)
	if (pSubService->open(ex)) {
		if (nextPath.empty())
			return pSubService;	
		return pSubService->open(ex, nextPath);
	}

	// service doesn't exist (and no children possible here!)
	if (it != _services.end() && ex.code() == Exception::APPLICATION) {
		delete it->second;
		_services.erase(it);
	}
	return NULL;
}

bool Service::open(bool create) {
	if (_reference != LUA_REFNIL)
		return true;
	if (!create)
		return false;

	//// create environment

	// table environment
	lua_newtable(_pState);

	// metatable
	lua_newtable(_pState);

#if !defined(_DEBUG)
	// hide metatable
	lua_pushliteral(_pState, "change metatable of environment is prohibited");
	lua_setfield(_pState, -2, "__metatable");
#endif

	// set parent
	if (_pParent) {
		_pParent->open(true); // guarantee the creation of parent!
		lua_rawgeti(_pState, LUA_REGISTRYINDEX, _pParent->reference());
		// fill children of parent!
		Script::Collection(_pState,-1,"children");
		lua_pushstring(_pState, name.c_str());
		lua_pushvalue(_pState, -5);
		Script::FillCollection(_pState, 1);
		lua_pop(_pState, 1); // remove children collection
	} else
		lua_pushvalue(_pState, LUA_GLOBALSINDEX);
	lua_setfield(_pState,-2,"super");

	// set name
	lua_pushstring(_pState, name.c_str());
	lua_setfield(_pState, -2, "name");

	// set path
	lua_pushstring(_pState, path.c_str());
	lua_setfield(_pState, -2, "path");

	// set this
	lua_pushvalue(_pState,-2);
	lua_setfield(_pState, -2, "this");

	// set __index=Service::Index
	lua_pushcfunction(_pState,&Service::Index);
	lua_pushvalue(_pState,-3);
	lua_setfenv(_pState, -2);
	lua_setfield(_pState,-2,"__index");

	// to be able to call SCRIPT_CALLBACK
	lua_pushlightuserdata(_pState,this);
	lua_setfield(_pState,-2,"|this");
	lua_pushlightuserdata(_pState,(void*)&typeid(Service));
	lua_setfield(_pState,-2,"|type");

	// set metatable
	lua_setmetatable(_pState,-2);

	// create children collection (collector required here!)
	Script::Collection<Service>(_pState,-1,"children");
	lua_pop(_pState, 1);

	// create clients table
	Script::Collection<Clients>(_pState, -1, "clients");
	lua_pop(_pState, 1);

	// record in registry
	setReference(luaL_ref(_pState, LUA_REGISTRYINDEX));

	return true;
}

void Service::loadFile() {

	open(true);
	
	_ex.set(Exception::NIL);

	SCRIPT_BEGIN(_pState)

		lua_rawgeti(_pState, LUA_REGISTRYINDEX, _reference);
		if(luaL_loadfile(_pState,file.path().c_str())!=0) {
			SCRIPT_ERROR(_ex.set(Exception::SOFTWARE, Script::LastError(_pState)).error())
			lua_pop(_pState,1); // remove environment
			return;
		}

		// set environment
		lua_pushvalue(_pState, -2);
		lua_setfenv(_pState, -2);

		if(lua_pcall(_pState, 0,0, 0)==0) {
			SCRIPT_FUNCTION_BEGIN("onStart",_reference)
				SCRIPT_WRITE_STRING(path.c_str())
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
			_handler.startService(*this);
			SCRIPT_INFO("Application www", path, " loaded")
		} else {
			SCRIPT_ERROR(_ex.set(Exception::SOFTWARE, Script::LastError(_pState)).error());
			clearEnvironment();
		}

		lua_pop(_pState, 1);

	SCRIPT_END
}

void Service::close(bool full) {

	if (open(false)) {

		if (!_ex) { // loaded!
			_handler.stopService(*this);
			SCRIPT_BEGIN(_pState)
				SCRIPT_FUNCTION_BEGIN("onStop",_reference)
					SCRIPT_WRITE_STRING(path.c_str())
					SCRIPT_FUNCTION_CALL
				SCRIPT_FUNCTION_END
			SCRIPT_END
		}

		lua_rawgeti(_pState, LUA_REGISTRYINDEX, _reference);
		clearEnvironment();
		if (full) {
			// Delete environment
			if (lua_getmetatable(_pState, -1)) {
				lua_getfield(_pState, -1, "super");
				if (lua_istable(_pState, -1)) {
					Script::Collection(_pState, -1, "children");
					lua_pushstring(_pState, name.c_str());
					lua_pushnil(_pState);
					Script::FillCollection(_pState, 1);
					lua_pop(_pState, 1);
				}
				lua_pushnumber(_pState, 0);
				lua_setfield(_pState, -2, "|this");
				lua_pop(_pState, 2);
			}
			setReference(LUA_REFNIL);
		}
		lua_pop(_pState, 1);

		lua_gc(_pState, LUA_GCCOLLECT, 0);
	}

	_ex.set(Exception::NIL);
}

void Service::clearEnvironment() {
	// Clear environment
	lua_pushnil(_pState);  // first key 
	while (lua_next(_pState, -2) != 0) {
		// uses 'key' (at index -2) and 'value' (at index -1) 
		// remove the raw!
		lua_pushvalue(_pState, -2); // duplicate key
		lua_pushnil(_pState);
		lua_rawset(_pState, -5);
		lua_pop(_pState, 1);
	}
}

int Service::LoadFile(lua_State *pState) {
	// 1 - name

	SCRIPT_BEGIN(pState)

	const char* name(lua_tostring(pState, 1));
	if (!name) {
		SCRIPT_ERROR("loadFile must take a string argument")
		return 0;
	}
	if (FileSystem::IsFolder(name)) {
		SCRIPT_ERROR("loadFile can't load a folder")
		return 0;
	}

	if (!FileSystem::IsAbsolute(name)) {

		Service* pService((Service*)lua_touserdata(pState,lua_upvalueindex(1)));
		string path;

		while (pService) {
			Exception ex;
			String::Format(path,pService->_rootPath,pService->path,'/',name);
	
			if (FileSystem::Exists(path)) {
				if (luaL_loadfile(pState, path.c_str()) == 0) {
					lua_rawgeti(pState, LUA_REGISTRYINDEX, pService->reference());
					lua_setfenv(pState, -2);
					return 1;
				}
				SCRIPT_ERROR(Script::LastError(pState))
				return 0;
			}
			
			pService = pService->_pParent;
		}
		
	}

	// try pure relative or absolute
	if (luaL_loadfile(pState, name) == 0)
		return 1;
	
	SCRIPT_ERROR(Script::LastError(pState))

	SCRIPT_END
	return 0;
}

int Service::ExecuteFile(lua_State *pState) {
	// 1 - name

	int results(0);

	SCRIPT_BEGIN(pState)

		bool isRequire(lua_toboolean(pState, lua_upvalueindex(2)) ? true : false);
		if (isRequire) {
			const char* name(lua_tostring(pState, 1));
			if (!name) {
				SCRIPT_ERROR("require must take a string argument")
				return 0;
			}
			string ext;
			if (String::ICompare(FileSystem::GetExtension(name,ext),"lua")==0)
				results = LoadFile(pState);
		} else
			results = LoadFile(pState);

		if (results) {
			results = lua_gettop(pState)-results;
			lua_call(pState, 0, LUA_MULTRET);
			results = lua_gettop(pState)-results;
		} else if (isRequire) {
			// is require, try lib
			results = lua_gettop(pState);
			lua_getglobal(pState, "require");
			lua_pushvalue(pState, 1);
			lua_call(pState, 1, LUA_MULTRET);
			results = lua_gettop(pState)-results;
		}

	SCRIPT_END

	return results;
}

int Service::Item(lua_State *pState) {
	SCRIPT_CALLBACK(Service,service)
	// 1 => environment table
	// 2 => children not found
	// here it check the existing of the service
	if (lua_isstring(pState, 2)) {
		string name(lua_tostring(pState, 2));
		if (!name.empty()) {
			Exception ex;
			Service* pService(service.open(ex, name));
			if (pService) // otherwise means error service
				lua_rawgeti(pState, LUA_REGISTRYINDEX, pService->reference());
		}
	}
	SCRIPT_CALLBACK_RETURN
}


// Call when a key is not available in the service table
int Service::Index(lua_State *pState) {
	if (!lua_getmetatable(pState, 1))
		return 0;

	if (lua_isstring(pState, 2)) {
		const char* key = lua_tostring(pState, 2);
		
		// |data table request?
		if (strcmp(key, "mona") == 0) {

			lua_getglobal(pState, "mona");
			if (lua_istable(pState, -1)) {

				// copy mona table and its metatable and change environment for functions
				UInt32 count(0);
				do {
					// create a new mona table, individual for this servie
					lua_newtable(pState);

					lua_pushnil(pState);  // first key 
					while (lua_next(pState, -3) != 0) {
						// uses 'key' (at index -2) and 'value' (at index -1) 
						lua_pushvalue(pState, -2); // duplicate key
						if (lua_isfunction(pState, -2)) {
							lua_pushcfunction(pState, lua_tocfunction(pState, -2)); // new value
							lua_pushvalue(pState, LUA_ENVIRONINDEX);
							lua_setfenv(pState,-2);
						} else
							lua_pushvalue(pState, -2); // duplicate value
						lua_rawset(pState, -5);
						lua_pop(pState, 1);
					}
					++count;
				} while (lua_getmetatable(pState, -2));

				while (--count) {
					lua_setmetatable(pState, -3);
					lua_pop(pState, 1);
				}

				SCRIPT_FIX_RESULT
				return 1;
			}
			lua_pop(pState, 1);
		} else if (strcmp(key, "data") == 0) {
			lua_getfield(pState, LUA_REGISTRYINDEX, "|data");
			if (lua_istable(pState,-1)) {
				lua_replace(pState, -2); // replace first metatable

				lua_getfield(pState, 1, "path");
				const char* path(lua_tostring(pState, -1));
				lua_pop(pState, 1);

				if (path) {  // else return |data
					String::ForEach forEach([pState](UInt32 index,const char* value){
						lua_getfield(pState,-1, value);
						if (lua_isnil(pState, -1)) {
							lua_pop(pState, 1);
							lua_newtable(pState);
							lua_setfield(pState, -2, value);
							lua_getfield(pState, -1, value);
						}
						lua_replace(pState, -2);
						return true;
					});
					String::Split(path, "/", forEach,String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
				}

				SCRIPT_FIX_RESULT
				return 1;
			}
			lua_pop(pState, 1);
		} else if (strcmp(key, "dofile") == 0) {

			lua_getfield(pState, -1, "|this");
			if (lua_isuserdata(pState, -1)) {
				lua_pushboolean(pState, false); // require = false
				lua_pushcclosure(pState, &Service::ExecuteFile, 2);
				SCRIPT_FIX_RESULT
				return 1;
			}
			lua_pop(pState, 1);
			
		} else if (strcmp(key, "require") == 0) {

			lua_getfield(pState, -1, "|this");
			if (lua_isuserdata(pState, -1)) {
				lua_pushboolean(pState, true); // require = true
				lua_pushcclosure(pState, &Service::ExecuteFile, 2);
				SCRIPT_FIX_RESULT
				return 1;
			}
			lua_pop(pState, 1);
			
		} else if (strcmp(key, "loadfile") == 0) {

			lua_getfield(pState, -1, "|this");
			if (lua_isuserdata(pState, -1)) {
				lua_pushcclosure(pState, &Service::LoadFile, 1);
				SCRIPT_FIX_RESULT
				return 1;
			}
			lua_pop(pState, 1);
			
		} else {
			// search in metatable (contains super, children, path, name, this, clients, ...)
			lua_getfield(pState, -1, key);
			if (!lua_isnil(pState, -1)) {
				lua_replace(pState, -2); // replace first metatable
				SCRIPT_FIX_RESULT
				return 1;
			}
			lua_pop(pState, 1);
		}
	}
	
	// search in parent (inheriting)
	lua_getfield(pState, -1, "super");
	lua_replace(pState,-2);
	if (lua_isnil(pState, -1))
		return 1; // no parent (returns nil)

	// search in parent
	lua_pushvalue(pState, 2);
	lua_gettable(pState,-2);
	lua_replace(pState,-2); // replace parent by result
	return 1;
}
