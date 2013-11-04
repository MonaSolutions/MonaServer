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

#pragma once

#include "Mona/FileWatcher.h"
#include "Script.h"

class Service;
class ServiceRegistry {
public:
	virtual void startService(Service& service){}
	virtual void stopService(Service& service){}
	virtual void addServiceFunction(Service& service,const std::string& name){}
	virtual void clearService(Service& service){}
};


class Service : public Mona::FileWatcher {
public:
	Service(lua_State* pState,const std::string& path,ServiceRegistry& registry);
	virtual ~Service();

	static void InitGlobalTable(lua_State *pState);
	static void StartVolatileObjectsRecording(lua_State* pState);
	static void StopVolatileObjectsRecording(lua_State* pState);

	Service*	get(const std::string& path);

	bool		refresh();
	lua_State*	open();

	const std::string	path;
	Mona::UInt32		count;
	const std::string	lastError;
private:
	
	bool		open(bool create);
	void		load();
	void		clear();

	static void	InitGlobalTable(lua_State* pState,bool pushMetatable);
	static int	Index(lua_State* pState);
	static int  NewIndex(lua_State* pState);

	bool						_running;
	lua_State*					_pState;
	bool						_deleting;
	std::vector<std::string>	_packages;

	std::map<std::string,Service*>	_services;
	ServiceRegistry&				_registry;

	static bool						_VolatileObjectsRecording;
	static std::thread::id			_VolatileObjectsThreadRecording;
};

inline void Service::InitGlobalTable(lua_State* pState) {
	InitGlobalTable(pState,false);
}

inline bool Service::refresh() {
	return FileWatcher::watch();
}
