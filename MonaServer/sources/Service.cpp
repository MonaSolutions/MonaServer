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

Service::Service(lua_State* pState, ServiceHandler& handler) : _created(false), _packages(1),Expirable(this), _handler(handler), _pState(pState), FileWatcher(handler.wwwPath(),"main.lua"), _loaded(false) {
	_packages[0].assign("www");
}

Service::Service(lua_State* pState, const string& path, ServiceHandler& handler) : _created(false), Expirable(this), _handler(handler), path(path), _pState(pState), FileWatcher(handler.wwwPath(),path,"main.lua"), _loaded(false) {
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

Service* Service::open(const string& path, Expirable<Service>& expirableService) {
	if (!_created)
		open(); // to guarantee that every father are opened at less one time!

	// remove first '/'
	string name;
	if(!path.empty())
		name.assign(path[0] == '/' ? path.substr(1, path.size() - 1) : path);

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
		return pSubService->open(nextPath, expirableService);

	if (pSubService->watchFile() || FileSystem::Exists(pSubService->filePath.directory())) { // if file or folder exists, return the service
		pSubService->shareThis(expirableService);
		pSubService->open();
		return pSubService;
	}

	// service doesn't exist (and no children possible here!)
	if (it != _services.end()) {
		delete it->second;
		_services.erase(it);
	}
	return NULL;
}


int Service::Item(lua_State *pState) {
	// 1 => children table
	// 2 => key not found
	// here it check the existing of the service
	const char* name = lua_tostring(pState, 2);
	if (!name)
		return 0;
	Service* pService = Script::GetCollector<Service>(pState,1);
	if (!pService)
		return 0;
	Expirable<Service> expirableService;
	if (!pService->open(name, expirableService))
		return 0;
	return 1;
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
				String::ForEach forEach([pState](const char* value){
					lua_getfield(pState,-1, value);
					if (lua_isnil(pState, -1)) {
						lua_pop(pState, 1);
						lua_newtable(pState);
						lua_setfield(pState, -2, value);
						lua_getfield(pState, -1, value);
					}
					lua_replace(pState, -2);
				});
				String::Split(path, "/", forEach,String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
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
	if (lua_isnil(pState, -1))
		return 1; // no parent (returns nil)

	lua_pushvalue(pState, 2);
	lua_gettable(pState,-2);
	lua_replace(pState,-2); // replace parent by result
	return 1;
}


bool Service::open(bool create) {
	_created = create;

	// get global metatable
	lua_getmetatable(_pState, LUA_GLOBALSINDEX);
	lua_pushvalue(_pState, LUA_GLOBALSINDEX);
	lua_getmetatable(_pState, LUA_GLOBALSINDEX);

	for (const string& value : _packages) {

		const char* package = value.c_str();
		
		// get children collection to parent (can be _G, no collector required here!)
		Script::Collection(_pState,-2,"children");
		
		lua_getfield(_pState, -1, package);
		if (!lua_istable(_pState, -1)) {
			if (!create) {
				lua_pop(_pState, 5);
				return false;
			}
			lua_pop(_pState, 1); // because is null!

			// table environment
			lua_newtable(_pState);

			lua_pushvalue(_pState, -2); // children collection

			// set child in children table of metatable of parent
			lua_pushstring(_pState, package);
			lua_pushvalue(_pState, -3); // table environment

			Script::FillCollection(_pState,1, lua_objlen(_pState, -3)+1);
			lua_pop(_pState, 1); // remove children
		}
		lua_replace(_pState, -2); // remove children	

		if (!lua_getmetatable(_pState, -1)) {

			//// create environment

			// metatable
			lua_newtable(_pState);

#if !defined(_DEBUG)
			// hide metatable
			lua_pushstring(_pState, "change metatable of environment is prohibited");
			lua_setfield(_pState, -2, "__metatable");
#endif

			// set parent
			lua_pushvalue(_pState,-4);
			lua_setfield(_pState,-2,"super");

			// set name
			lua_pushstring(_pState, package);
			lua_setfield(_pState, -2, "name");

			// set path
			lua_pushstring(_pState, path.c_str());
			lua_setfield(_pState, -2, "path");

			// set this
			lua_pushvalue(_pState,-2);
			lua_setfield(_pState, -2, "this");

			// set __index=Service::Index
			lua_pushcfunction(_pState,&Service::Index);
			lua_setfield(_pState,-2,"__index");

			// set metatable
			lua_pushvalue(_pState, -1);
			lua_setmetatable(_pState,-3);

			// create children collection (collector required here!)
			Script::Collection<Service>(_pState,-2,"children",this);
			lua_pop(_pState, 1);

			// in this scope the service table has been pushed
		}

		lua_replace(_pState, -3); // remove precedent metatable
		lua_replace(_pState, -3); // remove precedent service table
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
				if (lua_istable(_pState, -1)) {
					Script::Collection(_pState, -1, "children");
					lua_pushstring(_pState, _packages.back().c_str());
					lua_pushnil(_pState);
					Script::FillCollection(_pState, 1, lua_objlen(_pState,-3)-1);
					lua_pop(_pState, 1); // metatable of parent
				}
				lua_pop(_pState, 2);
			}
		} else
			clearEnvironment();

		lua_getmetatable(_pState, LUA_GLOBALSINDEX);
		lua_pushnil(_pState);
		lua_setfield(_pState, -2, "|env");

		lua_pop(_pState, 2);
		lua_gc(_pState, LUA_GCCOLLECT, 0);
	}
	_loaded = false;
	_created = !full;
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
