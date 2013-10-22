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

#include "Mona/Mona.h"
#include "Mona/Exceptions.h"
#include "Poco/UnWindows.h"

#if defined(POCO_WIN32_UTF8)
#define POCO_LPQUERY_SERVICE_CONFIG LPQUERY_SERVICE_CONFIGW
#else
#define POCO_LPQUERY_SERVICE_CONFIG LPQUERY_SERVICE_CONFIGA
#endif

namespace Mona {


class WinService {
public:
	enum Startup {
		AUTO_START,
		MANUAL_START,
		DISABLED
	};
	
	WinService(Exception& ex, const std::string& name);
	virtual ~WinService();

	const std::string& name() const {return _name;}

	void getPath(Exception& ex, std::string& path) const;

	void getDisplayName(Exception& ex, std::string& name) const;

	void registerService(Exception& ex, const std::string& path) { registerService(ex, path,_name); }
	void registerService(Exception& ex, const std::string& path,const std::string& displayName);
		/// Creates a Windows service with the executable specified by path
		/// and the given displayName.
		///
		/// Throws a ExistsException if the service has already been registered.
		
	void unregisterService(Exception& ex);
		/// Deletes the Windows service. 
		///
		/// Throws a NotFoundException if the service has not been registered.

	bool registered() const;
		/// Returns true if the service has been registered with the Service Control Manager.

	bool running(Exception& ex) const;
		/// Returns true if the service is currently running.
		
	void start(Exception& ex);
		/// Starts the service.
		/// Does nothing if the service is already running.
		///
		/// Throws a NotFoundException if the service has not been registered.

	void stop(Exception& ex);
		/// Stops the service.
		/// Does nothing if the service is not running.
		///
		/// Throws a NotFoundException if the service has not been registered.

	void setStartup(Exception& ex, Startup startup);
		/// Sets the startup mode for the service.
		
	Startup getStartup(Exception& ex) const;
		/// Returns the startup mode for the service.
		
	void setDescription(Exception& ex, const std::string& description);
		/// Sets the service description in the registry.
		
	void getDescription(Exception& ex, std::string& description) const;
		/// Returns the service description from the registry.

	static const int STARTUP_TIMEOUT;

protected:
	static const std::string REGISTRY_KEY;
	static const std::string REGISTRY_DESCRIPTION;

private:
	void open(Exception& ex) const;
	bool tryOpen() const;
	void close() const;
	POCO_LPQUERY_SERVICE_CONFIG config(Exception& ex) const;

	std::string       _name;
	SC_HANDLE         _scmHandle;
	mutable SC_HANDLE _svcHandle;
};


} // namespace Mona
