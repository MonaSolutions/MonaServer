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
#include "Mona/Application.h"
#include "Poco/Event.h"
#if defined(POCO_OS_FAMILY_WINDOWS)
#include "Poco/NamedEvent.h"
#endif
#include <iostream>

namespace Mona {

class ServerApplication: public Application {
public:
	ServerApplication();

	bool isInteractive() const;

#if defined(POCO_OS_FAMILY_UNIX) || (defined(POCO_OS_FAMILY_WINDOWS) && !defined(_WIN32_WCE))
	int run(int argc, char* argv[]);
#endif

	static void terminate();
		
protected:
	void waitForTerminationRequest();
#if defined(POCO_OS_FAMILY_UNIX) || (defined(POCO_OS_FAMILY_WINDOWS) && !defined(_WIN32_WCE))
	void defineOptions(Exception& ex, Options& options);
#endif

private:
	enum Action
	{
		SRV_RUN,
		SRV_REGISTER,
		SRV_UNREGISTER,
		SRV_HELP
	};

	Action			_action;

#if defined(POCO_VXWORKS)
	static Poco::Event _terminate;
#elif defined(POCO_OS_FAMILY_UNIX)
	void handlePidFile(const std::string& value);
	bool isDaemon(int argc, char** argv);
	void beDaemon();
#elif defined(POCO_OS_FAMILY_WINDOWS)
#if !defined(_WIN32_WCE)
	
	static BOOL __stdcall ConsoleCtrlHandler(DWORD ctrlType);
	static void __stdcall ServiceControlHandler(DWORD control);
	static void __stdcall ServiceMain(DWORD argc, LPTSTR* argv);


	bool hasConsole();
	bool isService();
	void beService();
	bool registerService(Exception& ex);
	bool unregisterService(Exception& ex);

	std::string _displayName;
	std::string _description;
	std::string _startup;


	static ServerApplication*	 _This;
	static Poco::Event           _terminated;
	static SERVICE_STATUS        _serviceStatus; 
	static SERVICE_STATUS_HANDLE _serviceStatusHandle; 
#endif // _WIN32_WCE
	static Poco::NamedEvent      _terminate;
#endif
};


} // namespace Mona
