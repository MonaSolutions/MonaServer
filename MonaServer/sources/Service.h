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

#pragma once

#include "Mona/FileWatcher.h"
#include "Mona/Expirable.h"
#include "Script.h"

class Service;
class ServiceHandler {
public:
	virtual void startService(Service& service){}
	virtual void stopService(Service& service){}
	virtual void assignData() {}
};


class Service : public Mona::FileWatcher, public Mona::Expirable<Service> {
public:
	Service(lua_State* pState, const std::string& path, ServiceHandler& handler);
	virtual ~Service();

	Service*			get(const std::string& path, Mona::Expirable<Service>& expirableService);
	lua_State*			open();

	const std::string	path;
	const std::string	lastError;
private:
	Service(lua_State* pState, const std::string& path, ServiceHandler& handler,bool init);

	bool		open(bool create);
	void		close(bool full);

	void		loadFile();
	void		clearFile() { close(false); }
	void		clearEnvironment();

	static int	Children(lua_State *pState);
	static int	CountChildren(lua_State *pState);
	static int	Index(lua_State* pState);

	bool						_loaded;
	lua_State*					_pState;
	std::vector<std::string>	_packages;

	std::map<std::string,Service*>	_services;
	ServiceHandler&					_handler;
};
