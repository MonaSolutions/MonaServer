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


#include "Mona/WinService.h"
#include "Mona/WinRegistryKey.h"
#include <thread>

#if !defined(_WIN32_WCE)

using namespace std;


namespace Mona {

const int WinService::STARTUP_TIMEOUT = 30000;
const string WinService::REGISTRY_KEY("SYSTEM\\CurrentControlSet\\Services\\");
const string WinService::REGISTRY_DESCRIPTION("Description");

WinService::WinService(const string& name) :_name(name), _svcHandle(0), _scmHandle(0) {
}


WinService::~WinService() {
	close();
	if (_scmHandle)
		CloseServiceHandle(_scmHandle);
}

bool WinService::open(Exception& ex,bool justManager) const {
	if (!_scmHandle && !(_scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
		ex.set(Exception::SYSTEM, "Cannot open Service Control Manager");
		return false;
	}
	if (!justManager && !_svcHandle && !(_svcHandle = OpenServiceA(_scmHandle, _name.c_str(), SERVICE_ALL_ACCESS))) {
		ex.set(Exception::SERVICE, "Service ", _name, " does not exist");
		return false;
	}
	return true;
}

void WinService::close() {
	if (_svcHandle) {
		CloseServiceHandle(_svcHandle);
		_svcHandle = 0;
	}
}


const string& WinService::getPath(Exception& ex, string& path) const {
	LPQUERY_SERVICE_CONFIGA pSvcConfig = config(ex);
	if (!pSvcConfig)
		return path;
	path.assign(pSvcConfig->lpBinaryPathName);
	LocalFree(pSvcConfig);
	return path;
}

const string& WinService::getDisplayName(Exception& ex, string& name) const {
	LPQUERY_SERVICE_CONFIGA pSvcConfig = config(ex);
	if (!pSvcConfig)
		return name;
	name.assign(pSvcConfig->lpDisplayName);
	LocalFree(pSvcConfig);
	return name;
}

bool WinService::registerService(Exception& ex,const string& path, const string& displayName) {
	close();
	if (!open(ex,true))
		return false;
	_svcHandle = CreateServiceA(
		_scmHandle,
		_name.c_str(),
		displayName.c_str(), 
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		path.c_str(),
		NULL, NULL, NULL, NULL, NULL);
	if (!_svcHandle)
		ex.set(Exception::SERVICE, "Cannot register service ", _name);
	return !ex;
}


bool WinService::unregisterService(Exception& ex) {
	if (!open(ex))
		return false;
	if (!DeleteService(_svcHandle))
		ex.set(Exception::SERVICE, "Cannot unregister service ", _name);
	return !ex;
}


bool WinService::registered(Exception& ex) const {
	Exception exc;
	bool result = open(exc);
	if (exc.code() == Exception::SYSTEM)
		ex = exc;
	return result;
}


bool WinService::running(Exception& ex) const {
	if (!open(ex))
		return false;
	SERVICE_STATUS ss;
	if (!QueryServiceStatus(_svcHandle, &ss))
		ex.set(Exception::SERVICE,"Cannot query service ",_name," status");
	return ss.dwCurrentState == SERVICE_RUNNING;
}

	
bool WinService::start(Exception& ex) {
	if (!open(ex))
		return false;
	if (!StartService(_svcHandle, 0, NULL)) {
		ex.set(Exception::SERVICE, "Cannot start service ", _name);
		return false;
	}

	SERVICE_STATUS svcStatus;
	int msecs = 0;
	while (msecs < STARTUP_TIMEOUT) {
		if (!QueryServiceStatus(_svcHandle, &svcStatus))
			break;
		if (svcStatus.dwCurrentState != SERVICE_START_PENDING)
			break;
		this_thread::sleep_for(chrono::milliseconds(250));
		msecs += 250;
	}
	if (!QueryServiceStatus(_svcHandle, &svcStatus))
		ex.set(Exception::SERVICE,"Cannot query status of starting service ", _name);
	else if (svcStatus.dwCurrentState != SERVICE_RUNNING)
		ex.set(Exception::SERVICE, "Service ", _name, " failed to start within a reasonable time");
	else
		return true;
	return false;
 }


bool WinService::stop(Exception& ex) {
	if (!open(ex))
		return false;
	SERVICE_STATUS svcStatus;
	if (!ControlService(_svcHandle, SERVICE_CONTROL_STOP, &svcStatus)) {
		ex.set(Exception::SERVICE, "Cannot stop service ", _name);
		return false;
	}
	return true;
}


bool WinService::setStartup(Exception& ex,Startup startup) {
	if (!open(ex))
		return false;
	DWORD startType;
	switch (startup) {
	case AUTO_START:
		startType = SERVICE_AUTO_START;
		break;
	case MANUAL_START:
		startType = SERVICE_DEMAND_START;
		break;
	case DISABLED:
		startType = SERVICE_DISABLED;
		break;
	default:
		startType = SERVICE_NO_CHANGE;
	}
	if (ChangeServiceConfig(_svcHandle, SERVICE_NO_CHANGE, startType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
		return true;
	ex.set(Exception::SERVICE, "Cannot change service ", _name, " startup mode");
	return false;
}

	
WinService::Startup WinService::getStartup(Exception& ex) const {
	LPQUERY_SERVICE_CONFIGA pSvcConfig = config(ex);
	if (!pSvcConfig)
		return UNKNOWN;
	Startup result;
	switch (pSvcConfig->dwStartType) {
		case SERVICE_AUTO_START:
		case SERVICE_BOOT_START:
		case SERVICE_SYSTEM_START:
			result = AUTO_START;
			break;
		case SERVICE_DEMAND_START:
			result = MANUAL_START;
			break;
		case SERVICE_DISABLED:
			result = DISABLED;
			break;
		default:
			ex.set(Exception::SERVICE, "Unknown startup mode ", pSvcConfig->dwStartType, " for the ", _name, " service");
			result = MANUAL_START;
	}
	LocalFree(pSvcConfig);
	return result;
}

bool WinService::setDescription(Exception& ex, const string& description) {
	string key(REGISTRY_KEY);
	key += _name;
	WinRegistryKey regKey(HKEY_LOCAL_MACHINE, key);
	return regKey.setString(ex,REGISTRY_DESCRIPTION, description);
}


const string& WinService::getDescription(Exception& ex,string& description) const {
	string key(REGISTRY_KEY);
	key += _name;
	WinRegistryKey regKey(HKEY_LOCAL_MACHINE, key, true);
	return regKey.getString(ex, REGISTRY_DESCRIPTION, description);
}


LPQUERY_SERVICE_CONFIGA WinService::config(Exception& ex) const {
	if (!open(ex))
		return NULL;
	int size = 4096;
	DWORD bytesNeeded;
	LPQUERY_SERVICE_CONFIGA pSvcConfig = (LPQUERY_SERVICE_CONFIGA)LocalAlloc(LPTR, size);
	if (!pSvcConfig) {
		ex.set(Exception::SERVICE, "Cannot allocate service config buffer");
		return NULL;
	}
	while (!QueryServiceConfigA(_svcHandle, pSvcConfig, size, &bytesNeeded)) {
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			LocalFree(pSvcConfig);
			size = bytesNeeded;
			pSvcConfig = (LPQUERY_SERVICE_CONFIGA)LocalAlloc(LPTR, size);
		} else {
			LocalFree(pSvcConfig);
			ex.set(Exception::SERVICE, "Cannot query service ", _name, " configuration");
			return NULL;
		}
	}
	return pSvcConfig;
}


} // namespace Mona


#endif // !defined(_WIN32_WCE)