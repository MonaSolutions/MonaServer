//
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

#include "Mona/WinService.h"
#include "Mona/WinRegistryKey.h"
#include "Mona/Exception.h"
#if defined(POCO_WIN32_UTF8)
#include "Poco/UnicodeConverter.h"
#endif
#include <thread>


#if !defined(_WIN32_WCE)

using namespace std;
using namespace Poco;

namespace Mona {

const int WinService::STARTUP_TIMEOUT = 30000;
const string WinService::REGISTRY_KEY("SYSTEM\\CurrentControlSet\\Services\\");
const string WinService::REGISTRY_DESCRIPTION("Description");

WinService::WinService(const string& name) :_name(name),_svcHandle(0) {
	_scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!_scmHandle)
		throw Exception(Exception::SERVICE, "Cannot open Service Control Manager");
}


WinService::~WinService() {
	close();
	CloseServiceHandle(_scmHandle);
}

void WinService::getPath(string& path) const {
	POCO_LPQUERY_SERVICE_CONFIG pSvcConfig = config();
#if defined(POCO_WIN32_UTF8)
	std::wstring upath(pSvcConfig->lpBinaryPathName);
	UnicodeConverter::toUTF8(upath, path);
#else
	path.assign(pSvcConfig->lpBinaryPathName);
#endif
	LocalFree(pSvcConfig);
}

void WinService::getDisplayName(string& name) const {
	POCO_LPQUERY_SERVICE_CONFIG pSvcConfig = config();
#if defined(POCO_WIN32_UTF8)
	wstring udispName(pSvcConfig->lpDisplayName);
	string dispName;
	UnicodeConverter::toUTF8(udispName, name);
#else
	name.assign(pSvcConfig->lpDisplayName);
#endif
	LocalFree(pSvcConfig);
}

void WinService::registerService(const std::string& path, const string& displayName) {
	close();
#if defined(POCO_WIN32_UTF8)
	wstring uname;
	UnicodeConverter::toUTF16(_name, uname);
	wstring udisplayName;
	UnicodeConverter::toUTF16(displayName, udisplayName);
	wstring upath;
	UnicodeConverter::toUTF16(path, upath);
	_svcHandle = CreateServiceW(
		_scmHandle,
		uname.c_str(),
		udisplayName.c_str(), 
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		upath.c_str(),
		NULL, NULL, NULL, NULL, NULL);
#else
	_svcHandle = CreateServiceA(
		_scmHandle,
		_name.c_str(),
		displayName.c_str(), 
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		_path.c_str(),
		NULL, NULL, NULL, NULL, NULL);
#endif
	if (!_svcHandle)
		throw Exception(Exception::SERVICE,"Cannot register service ", _name);
}


void WinService::unregisterService() {
	open();
	if (!DeleteService(_svcHandle))
		throw Exception(Exception::SERVICE,"Cannot unregister service ", _name);
}


bool WinService::registered() const {
	return tryOpen();
}


bool WinService::running() const {
	open();
	SERVICE_STATUS ss;
	if (!QueryServiceStatus(_svcHandle, &ss))
		throw Exception(Exception::SERVICE,"Cannot query service ",_name," status");
	return ss.dwCurrentState == SERVICE_RUNNING;
}

	
void WinService::start() {
	open();
	if (!StartService(_svcHandle, 0, NULL))
		throw Exception(Exception::SERVICE,"Cannot start service ", _name);

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
		throw Exception(Exception::SERVICE,"Cannot query status of starting service ", _name);
	else if (svcStatus.dwCurrentState != SERVICE_RUNNING)
		throw Exception(Exception::SERVICE, "Service ",_name," failed to start within a reasonable time");
 }


void WinService::stop() {
	open();
	SERVICE_STATUS svcStatus;
	if (!ControlService(_svcHandle, SERVICE_CONTROL_STOP, &svcStatus))
		throw Exception(Exception::SERVICE, "Cannot stop service ", _name);
}


void WinService::setStartup(Startup startup) {
	open();
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
	if (!ChangeServiceConfig(_svcHandle, SERVICE_NO_CHANGE, startType, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL))
		throw Exception(Exception::SERVICE,"Cannot change service ",_name," startup mode");
}

	
WinService::Startup WinService::getStartup() const {
	POCO_LPQUERY_SERVICE_CONFIG pSvcConfig = config();
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
		poco_debugger();
		result = MANUAL_START;
	}
	LocalFree(pSvcConfig);
	return result;
}

void WinService::setDescription(const string& description) {
	string key(REGISTRY_KEY);
	key += _name;
	WinRegistryKey regKey(HKEY_LOCAL_MACHINE, key);
	regKey.setString(REGISTRY_DESCRIPTION, description);
}


void WinService::getDescription(string& description) const {
	string key(REGISTRY_KEY);
	key += _name;
	WinRegistryKey regKey(HKEY_LOCAL_MACHINE, key, true);
	description = regKey.getString(REGISTRY_DESCRIPTION);
}


void WinService::open() const {
	if (!tryOpen())
		throw Exception(Exception::SERVICE,"Service ",_name," does not exist");
}

bool WinService::tryOpen() const {
	if (!_svcHandle) {
#if defined(POCO_WIN32_UTF8)
		wstring uname;
		UnicodeConverter::toUTF16(_name, uname);
		_svcHandle = OpenServiceW(_scmHandle, uname.c_str(), SERVICE_ALL_ACCESS);
#else
		_svcHandle = OpenServiceA(_scmHandle, _name.c_str(), SERVICE_ALL_ACCESS);
#endif
	}
	return _svcHandle != 0;
}


void WinService::close() const {
	if (_svcHandle) {
		CloseServiceHandle(_svcHandle);
		_svcHandle = 0;
	}
}


POCO_LPQUERY_SERVICE_CONFIG WinService::config() const {
	open();
	int size = 4096;
	DWORD bytesNeeded;
	POCO_LPQUERY_SERVICE_CONFIG pSvcConfig = (POCO_LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, size);
	if (!pSvcConfig)
		throw Exception(Exception::SERVICE,"Cannot allocate service config buffer");
	try {
#if defined(POCO_WIN32_UTF8)
		while (!QueryServiceConfigW(_svcHandle, pSvcConfig, size, &bytesNeeded)) {
#else
		while (!QueryServiceConfigA(_svcHandle, pSvcConfig, size, &bytesNeeded)) {
#endif
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
				LocalFree(pSvcConfig);
				size = bytesNeeded;
				pSvcConfig = (POCO_LPQUERY_SERVICE_CONFIG) LocalAlloc(LPTR, size);
			}
			else
				throw Exception(Exception::SERVICE,"Cannot query service ", _name, " configuration");
		}
	} catch (...) {
		LocalFree(pSvcConfig);
		throw;
	}
	return pSvcConfig;
}


} // namespace Mona


#endif // !defined(_WIN32_WCE)