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

#include "Service.h"
#include "MonaServer.h"
#include "Mona/String.h"
#include "Mona/FileSystem.h"


using namespace std;
using namespace Mona;


thread::id	Service::_VolatileObjectsThreadRecording;
bool		Service::_VolatileObjectsRecording(false);

Service::Service(lua_State* pState, const string& path, ServiceRegistry& registry) : path(path), _registry(registry), _pState(pState), FileWatcher(MonaServer::WWWPath + path + "/main.lua"), _running(false), _deleting(false), count(0) {
	
	String::Split("www" + path, "/", _packages, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);

	if(!refresh()) {
		open(true); // open even if no file
		lua_pop(_pState,1);
	}
}


Service::~Service() {
	// delete services children
	map<string,Service*>::const_iterator it;
	for(it=_services.begin();it!=_services.end();++it)
		delete it->second;
	// clean
	_deleting = true;
	clear();
}

// return NULL if "services folder" doesn't exist
Service* Service::get(const string& path) {
	if(path.empty()) {
		refresh();
		return this;
	}
	string name = path.substr(1,path.size()-1);

	size_t pos = name.find('/');

	string nextPath;
	if(pos!=string::npos)
		nextPath = name.substr(pos,name.size()-pos);

	// name folder
	name = name.substr(0,pos);
	
	// Folder exists?
	string directory(FileWatcher::path);
	FileSystem::MakeDirectory(directory);
	directory.append(name);
	bool exists = FileSystem::IsDirectory(directory) && FileSystem::Exists(directory);

	map<string,Service*>::iterator it = _services.lower_bound(name);
	
	if(it!=_services.end() && it->first==name) {
		// Service exists already
		if(exists)
			return it->second->get(nextPath);
		if(it->second->count==0) {
			delete it->second;
			_services.erase(it);
		}
		return NULL;
	}
	if(!exists)
		return NULL; // Service doesn't exist and folder too
	// Service does't exist but folder exists
	if(it!=_services.begin())
		--it;

	if(name == "__index" || name == "__metatable")
		CRITIC("You should never called a service '__index' or '__metatable', script behavior could become strange quickly!");

	Service* pService = new Service(_pState,this->path+"/"+name,_registry);
	_services.insert(it,pair<string,Service*>(name,pService));
	return pService->get(nextPath);
}


int Service::Index(lua_State *pState) {
	string key = lua_tostring(pState,2);
	lua_getmetatable(pState,1);
	
	if(key != "__index" && key != "__metatable" && key != "__newindex") {
		// in metatable? (metatable contains children)
		lua_getfield(pState,-1,key.c_str());
		if(!lua_isnil(pState,-1)) {
			lua_replace(pState,-2);
			return 1;
		}
		lua_pop(pState,1);
		
		// the service exists, but not created actually?
		lua_getfield(pState,-1,"//service");
		if(lua_islightuserdata(pState,-1)) {
			Service* pService = (Service*)lua_touserdata(pState,-1);
			lua_pop(pState,1);
			if(pService) {
				pService = pService->get("/"+key);
				if(pService) {
					lua_getfield(pState,-1,key.c_str());
					if(!lua_isnil(pState,-1)) {
						lua_replace(pState,-2);
						return 1;
					}
					lua_pop(pState,1);
				}
			}
		} else
			lua_pop(pState,1);
		
	}

	// search parent
	lua_getfield(pState,-1,"//parent");
	lua_replace(pState,-2);
	if(lua_isnil(pState,-1))
		return 1; // no parent

	// search key in parent
	lua_getfield(pState,-1,key.c_str());
	lua_replace(pState,-2);
	return 1;
}

int Service::NewIndex(lua_State *pState) {
	if(lua_isstring(pState,2)) {
		string key = lua_tostring(pState,2);
		if(lua_isfunction(pState,3)) {
			lua_getfield(pState,1,"//service");
			if(lua_islightuserdata(pState,-1)) {
				Service* pService = (Service*)lua_touserdata(pState,-1);
				pService->_registry.addServiceFunction(*pService,key);
			}
			lua_pop(pState,1);
		}
		if (_VolatileObjectsRecording && _VolatileObjectsThreadRecording == std::this_thread::get_id()) {
			if(lua_getmetatable(pState,3)!=0) { // stay
				// Is Mona object?
				lua_getfield(pState,-1,"__this"); // stay
				if(!lua_isnil(pState,-1)) {

					// persistent?
					lua_getfield(pState,-2,"//running"); // stay
					if(lua_isnil(pState,-1)) {

						lua_getfield(pState,-3,"__gcThis"); // has destructor?
						if(lua_isnil(pState,-1)) {

							// volatile object 
							lua_getmetatable(pState,LUA_GLOBALSINDEX); // stay
							lua_getfield(pState,-1,"//volatileObjects"); // stay

							lua_pushvalue(pState,1); // environment
							lua_rawget(pState,-2); // search environment key in "volatileObjects"
							if(lua_isnil(pState,-1)) {
								lua_pop(pState,1);
								lua_newtable(pState); // stay
								lua_pushvalue(pState,1);
								lua_pushvalue(pState,-2);
								lua_rawset(pState,-4); // add environment key in "volatileObjects"
							}
							lua_pushvalue(pState,2); // key
							lua_pushnumber(pState,1); // value, useless
							lua_rawset(pState,-3);
							lua_pop(pState,3);
						}
						lua_pop(pState,1);
					}
					lua_pop(pState,1);

				}
				lua_pop(pState,2);
			}
		}
	}
	lua_rawset(pState,1); // consumes key and value
	return 0;
}

void Service::StartVolatileObjectsRecording(lua_State* pState) {
	_VolatileObjectsRecording = true;
	_VolatileObjectsThreadRecording = this_thread::get_id();
	InitGlobalTable(pState,true);
	lua_newtable(pState);
	lua_setfield(pState,-2,"//volatileObjects");
	lua_pop(pState,1);
}

void Service::StopVolatileObjectsRecording(lua_State* pState) {
	if(_VolatileObjectsRecording) {
		_VolatileObjectsRecording = false;
		// get volatile objects
		lua_getmetatable(pState,LUA_GLOBALSINDEX); // stay
		lua_getfield(pState,-1,"//volatileObjects"); // stay
		// erase volatile objects
		lua_pushnil(pState);  // first key 
		while (lua_next(pState, -2) != 0) {
			// key = environment (-2), value = keys (-1)
			lua_pushnil(pState);  // first key 
			while (lua_next(pState, -2) != 0) {
				// key = key (-2), value = useless (-1)
				lua_pushnil(pState);
				lua_setfield(pState,-5,lua_tostring(pState,-3));
				lua_pop(pState,1);
			}
			lua_pop(pState,1);
		}
		lua_pop(pState,1);
		lua_pushnil(pState);
		lua_setfield(pState,-2,"//volatileObjects");
		lua_pop(pState,1);
	}
}

void Service::InitGlobalTable(lua_State* pState,bool pushMetatable) {
	// metatable of _G
	if(lua_getmetatable(pState,LUA_GLOBALSINDEX)!=0) {
		if(!pushMetatable)
			lua_pop(pState,1);
		return;
	}
	// global metatable
	lua_newtable(pState);

	// hide metatable
	lua_pushstring(pState,"change metatable of global environment is prohibited");
	lua_setfield(pState,-2,"__metatable");

	lua_pushcfunction(pState,&Service::Index);
	lua_setfield(pState,-2,"__index");

	lua_pushcfunction(pState,&Service::NewIndex);
	lua_setfield(pState,-2,"__newindex");

	if(pushMetatable)
		lua_pushvalue(pState,-1);
	lua_setmetatable(pState,LUA_GLOBALSINDEX);
}

lua_State* Service::open() {
	if(!_running)
		return NULL;

	InitGlobalTable(_pState,true);
	bool result = open(true);
	lua_setfield(_pState,-2,"//env");
	lua_pop(_pState,1);

	return result ? _pState : NULL;
}

bool Service::open(bool create) {

	lua_pushvalue(_pState,LUA_GLOBALSINDEX); // _G
	const char* precPackage=NULL;

	for (const string& value : _packages) {
		
		lua_getmetatable(_pState,-1);

		const char* package = value.c_str();

		lua_getfield(_pState,-1,package);
		if(!lua_istable(_pState,-1)) {
			if(!create) {
				lua_pop(_pState,4);
				return false;
			}
			lua_pop(_pState,1);

			// table environment
			lua_newtable(_pState);

			// set child in parent metatable
			lua_pushvalue(_pState,-1);
			lua_setfield(_pState,-3,package);

			// metatable
			lua_newtable(_pState);

			// set parent
			lua_pushvalue(_pState,-4);
			lua_setfield(_pState,-2,"//parent");
			lua_pushvalue(_pState,-4);
			lua_setfield(_pState,-2,"super");
			if(precPackage) {
				lua_pushvalue(_pState,-4);
				lua_setfield(_pState,-2,precPackage);
			}

			// set service
			lua_pushlightuserdata(_pState,this);
			lua_setfield(_pState,-2,"//service");

			// set self
			lua_pushvalue(_pState,-2);
			lua_setfield(_pState,-2,package);

			// set __index=Service::Index
			lua_pushcfunction(_pState,&Service::Index);
			lua_setfield(_pState,-2,"__index");

			// hide metatable
			lua_pushstring(_pState,"change metatable of environment is prohibited");
			lua_setfield(_pState,-2,"__metatable");

			// to manage volatile objects
			lua_pushcfunction(_pState,&Service::NewIndex);
			lua_setfield(_pState,-2,"__newindex");

			// set metatable
			lua_setmetatable(_pState,-2);

		}
		lua_replace(_pState,-2);
		lua_replace(_pState,-2);
		precPackage = package;
	}

	return true;
}

void Service::load() {

	InitGlobalTable(_pState,true);
	open(true);
	lua_pushvalue(_pState,-1);
	lua_setfield(_pState,-3,"//env");
	lua_replace(_pState,-2);

	(string&)lastError = "";

	if(luaL_loadfile(_pState,FileWatcher::path.c_str())!=0) {
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
			_running=true;
			
			SCRIPT_FUNCTION_BEGIN("onStart")
				SCRIPT_WRITE_STRING(path.c_str())
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
			_registry.startService(*this);
			SCRIPT_INFO("Application www",path,"/main.lua loaded")
		} else {
			const char* error = Script::LastError(_pState);
			SCRIPT_ERROR(error)
			(string&)lastError = error;
			// Clear environment
			lua_pushnil(_pState);  // first key 
			while (lua_next(_pState, -2) != 0) {
				// uses 'key' (at index -2) and 'value' (at index -1) 
				// remove the raw!
				lua_pushnil(_pState);
				lua_setfield(_pState,-4,lua_tostring(_pState,-3));
				lua_pop(_pState,1);
			}
		}
	SCRIPT_END

	lua_pop(_pState,1);
}


void Service::clear() {
	
	_running=false;
	(string&)lastError = "";

	InitGlobalTable(_pState,true);

	if(open(false)) {
		lua_pushvalue(_pState,-1);
		lua_setfield(_pState,-2,"//env");
		
		_registry.stopService(*this);
		SCRIPT_BEGIN(_pState)
			SCRIPT_FUNCTION_BEGIN("onStop")
				SCRIPT_WRITE_STRING(path.c_str())
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END

		if(_deleting) {
			// Delete environment
			lua_getmetatable(_pState,-1);

			// erase service
			lua_pushnil(_pState);
			lua_setfield(_pState,-2,"//service");

			lua_getfield(_pState,-1,"//parent");

			if(!lua_isnil(_pState,-1)) {
				lua_getmetatable(_pState,-1);
				lua_pushnil(_pState);
				lua_setfield(_pState,-2,(*(_packages.begin()+(_packages.size()-1))).c_str());
				lua_pop(_pState,1);
			}
			lua_pop(_pState,2);
		} else {
			// Clear environment
			lua_pushnil(_pState);  // first key 
			while (lua_next(_pState, -2) != 0) {
				// uses 'key' (at index -2) and 'value' (at index -1) 
				// remove the raw!
				lua_pushnil(_pState);
				lua_setfield(_pState,-4,lua_tostring(_pState,-3));
				lua_pop(_pState,1);
			}
		}

		lua_pop(_pState,1);
	}
	lua_pop(_pState,1);
	_registry.clearService(*this);
	lua_gc(_pState, LUA_GCCOLLECT, 0);
}
