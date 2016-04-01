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


#include "Mona/ServerApplication.h"
#include "Mona/Exceptions.h"
#include "Mona/HelpFormatter.h"
#if defined(_WIN32)
#include "Mona/WinService.h"
#include "Mona/WinRegistryKey.h"
#else
#include "Mona/Process.h"
#include "Mona/FileSystem.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fstream>
#endif
#include "Mona/Logs.h"

using namespace std;

namespace Mona {


ServerApplication*		ServerApplication::_PThis(NULL);
TerminateSignal			ServerApplication::_TerminateSignal;


#if defined(_WIN32)


static SERVICE_STATUS			_ServiceStatus;
static SERVICE_STATUS_HANDLE	_ServiceStatusHandle(0);

void  ServerApplication::ServiceControlHandler(DWORD control) {
	switch (control) {
	case SERVICE_CONTROL_STOP:
	case SERVICE_CONTROL_SHUTDOWN:
		_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(_ServiceStatusHandle, &_ServiceStatus);
		_TerminateSignal.set();
		return;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	}
	SetServiceStatus(_ServiceStatusHandle, &_ServiceStatus);
}


BOOL ServerApplication::ConsoleCtrlHandler(DWORD ctrlType) {
	switch (ctrlType) {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
		_TerminateSignal.set();
		return TRUE;
	default:
		return FALSE;
	}
}

void ServerApplication::ServiceMain(DWORD argc, LPTSTR* argv) {

	_PThis->setBoolean("application.runAsService", true);
	_PThis->_isInteractive = false;

	memset(&_ServiceStatus, 0, sizeof(_ServiceStatus));
	_ServiceStatusHandle = RegisterServiceCtrlHandlerA("", ServiceControlHandler);
	if (!_ServiceStatusHandle) {
		FATAL_ERROR("Cannot register service control handler");
		return;
	}
	_ServiceStatus.dwServiceType = SERVICE_WIN32;
	_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	_ServiceStatus.dwWin32ExitCode = 0;
	_ServiceStatus.dwServiceSpecificExitCode = 0;
	_ServiceStatus.dwCheckPoint = 0;
	_ServiceStatus.dwWaitHint = 0;
	SetServiceStatus(_ServiceStatusHandle, &_ServiceStatus);

#if !defined(_DEBUG)
	try {
#endif

		if (_PThis->init(argc, const_cast<LPCSTR*>(argv))) {
			_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
			SetServiceStatus(_ServiceStatusHandle, &_ServiceStatus);
			int rc = _PThis->main(_TerminateSignal);
			_ServiceStatus.dwWin32ExitCode = rc ? ERROR_SERVICE_SPECIFIC_ERROR : 0;
			_ServiceStatus.dwServiceSpecificExitCode = rc;
		}

#if !defined(_DEBUG)
	} catch (exception& ex) {
		FATAL(ex.what());
		_ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		_ServiceStatus.dwServiceSpecificExitCode = EXIT_SOFTWARE;
	} catch (...) {
		FATAL("Unknown error");
		_ServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		_ServiceStatus.dwServiceSpecificExitCode = EXIT_SOFTWARE;
	}
#endif

	_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(_ServiceStatusHandle, &_ServiceStatus);
}


int ServerApplication::run(int argc, const char** argv) {
#if !defined(_DEBUG)
	try {
#endif
		if (!hasConsole() && isService())
			return 0;

		SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

		if (!init(argc, argv))
			return EXIT_OK;
		Exception ex;
		if (hasArgument("registerService")) {
			bool success = false;
			EXCEPTION_TO_LOG(success = registerService(ex), "RegisterService")
			if (success)
				NOTE("The application has been successfully registered as a service");
			return EXIT_OK;
		}
		if (hasArgument("unregisterService")) {
			bool success = false;
			EXCEPTION_TO_LOG(success = unregisterService(ex), "UnregisterService")
			if (success)
				NOTE("The application is no more registered as a service");
			return EXIT_OK;
		}
		return main(_TerminateSignal);
#if !defined(_DEBUG)
	} catch (exception& ex) {
		FATAL(ex.what());
		return EXIT_SOFTWARE;
	} catch (...) {
		FATAL("Unknown error");
		return EXIT_SOFTWARE;
	}
#endif
}


bool ServerApplication::isService() {
	SERVICE_TABLE_ENTRY svcDispatchTable[2];
	svcDispatchTable[0].lpServiceName = "";
	svcDispatchTable[0].lpServiceProc = ServiceMain;
	svcDispatchTable[1].lpServiceName = NULL;
	svcDispatchTable[1].lpServiceProc = NULL; 
	return StartServiceCtrlDispatcherA(svcDispatchTable) != 0; 
}

bool ServerApplication::hasConsole() {
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	return hStdOut != INVALID_HANDLE_VALUE && hStdOut != NULL;
}


bool ServerApplication::registerService(Exception& ex) {
	WinService service(file().name());
	if (_displayName.empty())
		service.registerService(ex,file().path());
	else
		service.registerService(ex, file().path(), _displayName);
	if (ex)
		return false;
	if (_startup == "auto")
		service.setStartup(ex,WinService::AUTO_START);
	else if (_startup == "manual")
		service.setStartup(ex,WinService::MANUAL_START);
	if (!_description.empty())
		service.setDescription(ex, _description);
	return true;
}


bool ServerApplication::unregisterService(Exception& ex) {
	WinService service(file().name());
	return service.unregisterService(ex);
}

void ServerApplication::defineOptions(Exception& ex,Options& options) {

	options.add(ex, "registerService", "r", "Register the application as a service.");

	options.add(ex, "unregisterService", "u", "Unregister the application as a service.");

	options.add(ex, "name", "n", "Specify a display name for the service (only with /registerService).")
		.argument("name")
		.handler([this](Exception& ex, const string& value) { _displayName = value; return true; });

	options.add(ex, "description", "d", "Specify a description for the service (only with /registerService).")
		.argument("text")
		.handler([this](Exception& ex, const string& value) { _description = value; return true; });

	options.add(ex, "startup", "s", "Specify the startup mode for the service (only with /registerService).")
		.argument("automatic|manual")
		.handler([this](Exception& ex, const string& value) {_startup = String::ICompare(value, "auto", 4) == 0 ? "auto" : "manual"; return true; });

	Application::defineOptions(ex, options);
}

#else

//
// Unix specific code
//

int ServerApplication::run(int argc, const char** argv) {
	int result(EXIT_OK);
#if !defined(_DEBUG)
	try {
#endif
		bool runAsDaemon = isDaemon(argc, argv);
		if (runAsDaemon)
			beDaemon();
		if(init(argc, argv)) {
			if (runAsDaemon) {
				int rc = chdir("/");
				if (rc != 0)
					result = EXIT_OSERR;
			}
			if(result==EXIT_OK) {
				result = main(_TerminateSignal);
			}
		}
#if !defined(_DEBUG)
	} catch (exception& ex) {
		FATAL( ex.what());
		result = EXIT_SOFTWARE;
	} catch (...) {
		FATAL("Unknown error");
		result = EXIT_SOFTWARE;
	}
#endif
	if (!_pidFile.empty()) {
		Exception ex;
		FileSystem::Delete(ex,_pidFile);
		ERROR("pid file deletion, ",ex.error());
	}
	return result;
}



bool ServerApplication::isDaemon(int argc, const char** argv) {
	string option1("--daemon");
	string option2("-d");
	string option3("/daemon");
	string option4("/d");
	for (int i = 1; i < argc; ++i) {
		if (String::ICompare(option1,argv[i])==0 || String::ICompare(option2,argv[i])==0 || String::ICompare(option3,argv[i])==0 || String::ICompare(option4,argv[i])==0)
			return true;
	}
	return false;
}


void ServerApplication::beDaemon() {
	pid_t pid;
	if ((pid = fork()) < 0)
        FATAL_ERROR("Cannot fork daemon process");
	if (pid != 0)
		exit(0);
	
	setsid();
	umask(0);
	
	// attach stdin, stdout, stderr to /dev/null
	// instead of just closing them. This avoids
	// issues with third party/legacy code writing
	// stuff to stdout/stderr.
	FILE* fin  = freopen("/dev/null", "r+", stdin);
	if (!fin)
        FATAL_ERROR("Cannot attach stdin to /dev/null");
	FILE* fout = freopen("/dev/null", "r+", stdout);
	if (!fout)
        FATAL_ERROR("Cannot attach stdout to /dev/null");
	FILE* ferr = freopen("/dev/null", "r+", stderr);
	if (!ferr)
        FATAL_ERROR("Cannot attach stderr to /dev/null");

	setBoolean("application.runAsDaemon", true);
	_isInteractive=false;
}


void ServerApplication::defineOptions(Exception& ex, Options& options) {
    options.add(ex, "daemon", "d", "Run application as a daemon.")
        .handler([this](Exception& ex, const string& value) { setBoolean("application.runAsDaemon", true); return true; });

    options.add(ex, "pidfile", "p", "Write the process ID of the application to given file.")
		.argument("path")
        .handler([this](Exception& ex, const string& value) { handlePidFile(ex, value); return true; } );

    Application::defineOptions(ex, options);
}


void ServerApplication::handlePidFile(Exception& ex,const string& value) {
	ofstream ostr(value.c_str(), ios::out | ios::binary);
	if (!ostr.good()) {
		ex.set(Exception::FILE,"Cannot write PID to file ",value);
		return;
	}
    ostr << Process::Id() << endl;
	_pidFile = value;
}

#endif


} // namespace Mona
