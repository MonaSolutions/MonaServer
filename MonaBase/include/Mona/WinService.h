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

#include "Mona/Mona.h"
#include "Mona/Exceptions.h"
#if defined(_WIN32)
#include <Windows.h>
#endif

namespace Mona {


class WinService : public virtual Object {
public:
	enum Startup {
		UNKNOWN,
		AUTO_START,
		MANUAL_START,
		DISABLED
	};
	
	WinService(const std::string& name);
	virtual ~WinService();

	const std::string& name() const {return _name;}

	const std::string& getPath(Exception& ex, std::string& path) const;

	const std::string& getDisplayName(Exception& ex, std::string& name) const;

	bool registerService(Exception& ex, const std::string& path) { return registerService(ex,path, _name); }
	bool registerService(Exception& ex, const std::string& path, const std::string& displayName);
		/// Creates a Windows service with the executable specified by path
		/// and the given displayName.
		///
        /// Throws an exception if the service has already been registered.
		
	bool unregisterService(Exception& ex);
		/// Deletes the Windows service. 
		///
        /// Throws an exception if the service has not been registered.

	bool registered(Exception& ex) const;
		/// Returns true if the service has been registered with the Service Control Manager.

	bool running(Exception& ex) const;
		/// Returns true if the service is currently running.
		
	bool start(Exception& ex);
		/// Starts the service.
		/// Does nothing if the service is already running.
		///
        /// Throws an exception if the service has not been registered.

	bool stop(Exception& ex);
		/// Stops the service.
		/// Does nothing if the service is not running.
		///
        /// Throws an exception if the service has not been registered.

	bool setStartup(Exception& ex, Startup startup);
		/// Sets the startup mode for the service.
		
	Startup getStartup(Exception& ex) const;
		/// Returns the startup mode for the service.
		
	bool setDescription(Exception& ex, const std::string& description);
		/// Sets the service description in the registry.
		
	const std::string& getDescription(Exception& ex, std::string& description) const;
		/// Returns the service description from the registry.

	static const int STARTUP_TIMEOUT;

protected:
	static const std::string REGISTRY_KEY;
	static const std::string REGISTRY_DESCRIPTION;

private:
	bool open(Exception& ex,bool justManager=false) const;
	void close();

	LPQUERY_SERVICE_CONFIGA config(Exception& ex) const;

	std::string       _name;
	mutable SC_HANDLE _scmHandle;
	mutable SC_HANDLE _svcHandle;
};


} // namespace Mona
