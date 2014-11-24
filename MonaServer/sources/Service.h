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

#include "Mona/Client.h"
#include "Script.h"
#include "Mona/FileWatcher.h"


class Service;
class ServiceHandler {
public:
	virtual void startService(Service& service){}
	virtual void stopService(Service& service){}
};


class Service : private Mona::FileWatcher {
public:
	Service(lua_State* pState, const std::string& rootPath, ServiceHandler& handler);
	virtual ~Service();

	int					reference() const { return _reference; }

	Service*			open(Mona::Exception& ex);
	Service*			open(Mona::Exception& ex, const std::string& path);

	static int	Item(lua_State *pState);

	const std::string   name;
	const std::string	path;

private:
	Service(lua_State* pState,  const std::string& rootPath, Service& parent, const std::string& name, ServiceHandler& handler);

	static int  LoadFile(lua_State *pState);
	static int	ExecuteFile(lua_State *pState);

	bool		open(bool create);
	void		setReference(int reference);
	void		close(bool full);

	void		loadFile();
	void		clearFile() { close(false); }
	void		clearEnvironment();

	static int	Index(lua_State* pState);

	Mona::Exception				_ex;
	int							_reference;
	Service*					_pParent;
	lua_State*					_pState;
	Mona::Time					_lastCheck;


	std::map<std::string,Service*>	_services;
	const std::string&				_rootPath;
	ServiceHandler&					_handler;
};
