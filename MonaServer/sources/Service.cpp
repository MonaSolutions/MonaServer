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
#include "Mona/FileSystem.h"


using namespace std;
using namespace Mona;

Service::Service(lua_State* pState, const string& path, ServiceHandler& handler) : Expirable(this), _handler(handler), path(path), _pState(pState), FileWatcher(handler.wwwPath(),path,"main.lua"), _loaded(false) {
	String::Split("www" + path, "/", _packages, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
}

Service::~Service() {
	expire();
	// clean children
	for (auto& it : _services)
		delete it.second;
	// clean this
	close(true);
}

Service* Service::get(const string& path, Expirable<Service>& expirableService) {
	// remove first '/'
	string name = path[0] == '/' ? path.substr(1, path.size() - 1) : path;

	// substr first "service"
	size_t pos = name.find('/');
	string nextPath;
	if (pos != string::npos) {
		nextPath = name.substr(pos, name.size() - pos);
		name.resize(pos);
	}

	Service* pSubService(this);
	auto it = _services.end();
	if (!name.empty()) {
		it = _services.lower_bound(name);
		if (it == _services.end() || it->first != name)
			it = _services.emplace_hint(it, name, new Service(_pState, this->path + "/" + name, _handler)); // Service doesn't exists
		pSubService = it->second;
	}

	if (!nextPath.empty())
		return pSubService->get(nextPath, expirableService);

	if (pSubService->watchFile() || FileSystem::Exists(pSubService->filePath.directory())) { // if file or folder exists, return the service
		pSubService->shareThis(expirableService);
		return pSubService;
	}
	// service doesn't exist
	if (it != _services.end()) {
		delete it->second;
		_services.erase(it);
	}
	return NULL;
}


int Service::Children(lua_State *pState) {
	// 1 => table
	// 2 => key
	// here it check the existing of the service
	if (lua_isstring(pState, 2) && lua_getmetatable(pState, 1)) {
		lua_getfield(pState, -1, "|service");
		Service* pService = (Service*)lua_touserdata(pState, -1);
		lua_pop(pState, 2);
		const char* name = lua_tostring(pState, 2);
		Expirable<Service> expirableService;
		if (!pService || !(pService = pService->get(name, expirableService)))
			return 0;
		pService->open();
		return 1;
	}
	return 0;
}

int Service::CountChildren(lua_State *pState) {
	if (lua_getmetatable(pState, 1)) {
		lua_getfield(pState, -1, "|service");
		Service* pService = (Service*)lua_touserdata(pState, -1);
		if (pService) {
			lua_pushnumber(pState, pService->_services.size());
			return 1;
		}
	}
	return 0;
}

// Call when a key is not available in the service table
int Service::Index(lua_State *pState) {
	if (!lua_getmetatable(pState, 1))
		return 0;

	if (lua_isstring(pState, 2)) {
		const char* key = lua_tostring(pState, 2);
		
		// //data table request?
		if (strcmp(key, "data") == 0) {
			lua_getmetatable(pState, LUA_GLOBALSINDEX);
			lua_getfield(pState, -1, "|data");
			lua_replace(pState, -2);
			if (!lua_isnil(pState,-1)) {
				lua_replace(pState, -2);
				if (!lua_istable(pState, -1))
					return 1;
				string path;
				lua_getfield(pState, -3, "path");
				if (lua_isstring(pState, -1))
					path.assign(lua_tostring(pState, -1));
				lua_pop(pState, 1);
				if (path.empty())
					return 1; // return //data
				vector<string> values;
				String::Split(path, "/", values,String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
				for (const string& value : values) {
					lua_getfield(pState,-1, value.c_str());
					if (lua_isnil(pState, -1)) {
						lua_pop(pState, 1);
						lua_newtable(pState);
						lua_setfield(pState, -2, value.c_str());
						lua_getfield(pState, -1, value.c_str());
					}
					lua_replace(pState, -2);
				}
				return 1;
			}
			lua_pop(pState, 1);
		}
		
		// search in metatable (contains super, children, path, name, this,...)
		lua_getfield(pState, -1, key);
		if (!lua_isnil(pState, -1)) {
			lua_replace(pState, -2);
			return 1;
		}
		lua_pop(pState, 1);
	}
	
	// search in parent (inheriting)
	lua_getfield(pState, -1, "super");
	lua_replace(pState,-2);
	if(lua_isnil(pState,-1))
		return 1; // no parent (returns nil)
	
	lua_pushvalue(pState, LUA_GLOBALSINDEX);
	int result = lua_equal(pState, -1, -2);
	lua_pop(pState, 1);

	lua_pushvalue(pState, 2);
	lua_gettable(pState,-2);
	lua_replace(pState,-2); // replace parent by result
	return 1;
}

lua_State* Service::open() {
	lua_State* pResult = open(true) ? _pState : NULL;
	lua_pop(_pState, 1);
	return pResult;
}

bool Service::open(bool create) {
	// get global metatable
	lua_getmetatable(_pState, LUA_GLOBALSINDEX);
	lua_pushvalue(_pState, LUA_GLOBALSINDEX);
	lua_getmetatable(_pState, LUA_GLOBALSINDEX);

	for (const string& value : _packages) {
		
		// get children table
		lua_getfield(_pState, -1, "children");
		if (!lua_istable(_pState, -1)) {
			lua_pop(_pState, 1);
			// set an empty children table
			lua_newtable(_pState);
			lua_pushvalue(_pState, -1);
			lua_setfield(_pState, -3, "children");
		}

		const char* package = value.c_str();

		lua_getfield(_pState, -1, package);
		if (!lua_istable(_pState, -1)) {
			if (!create) {
				lua_pop(_pState, 5);
				return false;
			}
			lua_pop(_pState, 1); // because is null!

			// table environment
			lua_newtable(_pState);

			// set child in children table of metatable of parent
			lua_pushvalue(_pState, -1); // table environment
			lua_setfield(_pState,-3,package);
		}

		lua_remove(_pState, -2); // remove children	

		if (lua_getmetatable(_pState, -1)==0) {

			//// create environment

			// metatable
			lua_newtable(_pState);

#if !defined(_DEBUG)
			// hide metatable
			lua_pushstring(_pState, "change metatable of environment is prohibited");
			lua_setfield(_pState, -2, "__metatable");
#endif

			lua_pushvalue(_pState,-4);
			lua_setfield(_pState,-2,"super");

			// set name
			lua_pushstring(_pState, package);
			lua_setfield(_pState, -2, "name");

			// set path
			lua_pushstring(_pState, path.c_str());
			lua_setfield(_pState, -2, "path");

			// set this
			lua_pushlightuserdata(_pState, this);
			lua_setfield(_pState, -2, "this");

			// set children table
			lua_newtable(_pState);
			lua_newtable(_pState); // metatable
#if !defined(_DEBUG)
			lua_pushstring(_pState, "change metatable of map is prohibited");
			lua_setfield(_pState, -2, "__metatable");
#endif
			lua_pushcfunction(_pState, &Service::CountChildren);
			lua_setfield(_pState, -2, "__len");
			lua_pushcfunction(_pState,&Service::Children);
			lua_setfield(_pState, -2, "__call");
			lua_pushlightuserdata(_pState, this);
			lua_setfield(_pState, -2, "|service");
			lua_setmetatable(_pState, -2);
			lua_setfield(_pState, -2, "children");
			
			// set __index=Service::Index
			lua_pushcfunction(_pState,&Service::Index);
			lua_setfield(_pState,-2,"__index");

			// set metatable
			lua_pushvalue(_pState, -1);
			lua_setmetatable(_pState,-3);
			// in this scope the service table has been pushed
		}
		lua_remove(_pState, -3); // remove precedent metatable
		lua_remove(_pState, -3); // remove precedent service table
	}

	lua_pop(_pState, 1); // remove metatable
	lua_pushvalue(_pState, -1);
	lua_setfield(_pState, -3, "|env");
	lua_replace(_pState, -2);

	return true;
}

void Service::loadFile() {
	if (_loaded)
		return;
	open(true);
	
	(string&)lastError = "";

	if(luaL_loadfile(_pState,filePath.fullPath().c_str())!=0) {
		SCRIPT_BEGIN(_pState)
			const char* error = Script::LastError(_pState);
			SCRIPT_ERROR(error)
			(string&)lastError = error;
		SCRIPT_END
		lua_pop(_pState,1);
		return;
	}

	SCRIPT_BEGIN(_pState)

		lua_pushvalue(_pState,-2);
		lua_setfenv(_pState,-2);
		if(lua_pcall(_pState, 0,0, 0)==0) {
			_loaded=true;
			
			SCRIPT_FUNCTION_BEGIN("onStart")
				SCRIPT_WRITE_STRING(path.c_str())
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
			_handler.startService(*this);
			SCRIPT_INFO("Application www", path, " loaded")
		} else {
			(string&)lastError = Script::LastError(_pState);
			SCRIPT_ERROR(lastError);
			clearEnvironment();
		}
	SCRIPT_END

	lua_pop(_pState,1);
}

void Service::close(bool full) {

	(string&)lastError = "";
	if (open(false)) {

		if (_loaded) {
			_handler.stopService(*this);
			SCRIPT_BEGIN(_pState)
				SCRIPT_FUNCTION_BEGIN("onStop")
					SCRIPT_WRITE_STRING(path.c_str())
					SCRIPT_FUNCTION_CALL
				SCRIPT_FUNCTION_END
			SCRIPT_END
		}

		if (full) {
			// Delete environment
			if (lua_getmetatable(_pState, -1)) {
				lua_getfield(_pState, -1, "super");
				if (!lua_isnil(_pState, -1) && lua_getmetatable(_pState, -1)) {
					lua_getfield(_pState, -1, "children");
					lua_pushnil(_pState);
					lua_setfield(_pState, -2, (_packages[_packages.size() - 1]).c_str());
					lua_pop(_pState, 1); // metatable of parent
				}
				lua_pop(_pState, 2);
			}
		} else
			clearEnvironment();
		lua_pop(_pState, 1);
		lua_gc(_pState, LUA_GCCOLLECT, 0);
	}
	_loaded = false;
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
