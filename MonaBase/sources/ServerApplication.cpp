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


#include "Mona/ServerApplication.h"
#include "Mona/Logs.h"
#include "Mona/Exceptions.h"
#include "Mona/HelpFormatter.h"
#if !defined(POCO_VXWORKS)
#include "Poco/Process.h"
#include "Poco/NamedEvent.h"
#endif
#include "Poco/String.h"
#if defined(POCO_OS_FAMILY_UNIX) && !defined(POCO_VXWORKS)
#include "Poco/TemporaryFile.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <fstream>
#elif defined(POCO_OS_FAMILY_WINDOWS)
#if !defined(_WIN32_WCE)
#include "Mona/WinService.h"
#include "Mona/WinRegistryKey.h"
#endif
#include "Poco/UnWindows.h"
#include <cstring>
#endif


using namespace std;
using namespace Poco;

namespace Mona {


#if defined(POCO_OS_FAMILY_WINDOWS)
Poco::NamedEvent      ServerApplication::_terminate(Poco::ProcessImpl::terminationEventName(Poco::Process::id()));
#if !defined(_WIN32_WCE)
Poco::Event           ServerApplication::_terminated;
SERVICE_STATUS        ServerApplication::_serviceStatus; 
SERVICE_STATUS_HANDLE ServerApplication::_serviceStatusHandle = 0; 
#endif
#endif
#if defined(POCO_VXWORKS)
Poco::Event ServerApplication::_terminate;
#endif
ServerApplication* ServerApplication::_This(NULL);


ServerApplication::ServerApplication() : _action(SRV_RUN), _isInteractive(true) {
	_This = this;
#if defined(POCO_OS_FAMILY_WINDOWS)
#if !defined(_WIN32_WCE)
	memset(&_serviceStatus, 0, sizeof(_serviceStatus));
#endif
#endif
}


void ServerApplication::terminate() {
#if defined(POCO_OS_FAMILY_WINDOWS)
	_terminate.set();
#elif defined(POCO_VXWORKS)
	_terminate.set();
#else
	Poco::Process::requestTermination(Process::id());
#endif
}


#if defined(POCO_OS_FAMILY_WINDOWS)
#if !defined(_WIN32_WCE)


//
// Windows specific code
//
BOOL ServerApplication::ConsoleCtrlHandler(DWORD ctrlType) {
	switch (ctrlType)  { 
		case CTRL_C_EVENT: 
		case CTRL_CLOSE_EVENT: 
		case CTRL_BREAK_EVENT:
			terminate();
			return _terminated.tryWait(10000) ? TRUE : FALSE;
		default: 
			return FALSE; 
	}
}


void ServerApplication::ServiceControlHandler(DWORD control) {
	switch (control) { 
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
			terminate();
			_serviceStatus.dwCurrentState = SERVICE_STOP_PENDING;
			break;
		case SERVICE_CONTROL_INTERROGATE: 
			break; 
	} 
	SetServiceStatus(_serviceStatusHandle,  &_serviceStatus);
}



void ServerApplication::ServiceMain(DWORD argc, LPTSTR* argv) {

	_This->setBool("application.runAsService", true);
	_This->_isInteractive = false;

	_serviceStatusHandle = RegisterServiceCtrlHandlerA("", ServiceControlHandler);

	if (!_serviceStatusHandle) {
		FATAL("Cannot register service control handler");
		return;
	}

	_serviceStatus.dwServiceType             = SERVICE_WIN32; 
	_serviceStatus.dwCurrentState            = SERVICE_START_PENDING; 
	_serviceStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	_serviceStatus.dwWin32ExitCode           = 0; 
	_serviceStatus.dwServiceSpecificExitCode = 0; 
	_serviceStatus.dwCheckPoint              = 0; 
	_serviceStatus.dwWaitHint                = 0; 
	SetServiceStatus(_serviceStatusHandle, &_serviceStatus);

	try {
		if (_This->init(argc, argv)) {
			_serviceStatus.dwCurrentState = SERVICE_RUNNING;
			SetServiceStatus(_serviceStatusHandle, &_serviceStatus);
			int rc = _This->main();
			_serviceStatus.dwWin32ExitCode = rc ? ERROR_SERVICE_SPECIFIC_ERROR : 0;
			_serviceStatus.dwServiceSpecificExitCode = rc;
		}
	} catch (exception& ex) {
		FATAL(ex.what());
		_serviceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		_serviceStatus.dwServiceSpecificExitCode = EXIT_SOFTWARE;
	} catch (...) {
		FATAL("Unknown error");
		_serviceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		_serviceStatus.dwServiceSpecificExitCode = EXIT_SOFTWARE;
	}
	_serviceStatus.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(_serviceStatusHandle, &_serviceStatus);
}


void ServerApplication::waitForTerminationRequest() {
	SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
	_terminate.wait();
	_terminated.set();
}

int ServerApplication::run(int argc, char** argv) {
	try {
		if (!hasConsole() && isService())
			return 0;
		if (!init(argc, argv))
			return EXIT_OK;
		Exception ex;
		if (hasArgument("registerService")) {
			if (registerService(ex)) {
				if (!ex)
					WARN(ex.error());
				NOTE("The application has been successfully registered as a service");
			} else
				ERROR(ex.error());
			return EXIT_OK;
		}
		if (hasArgument("unregisterService")) {
			if (unregisterService(ex)) {
				if (!ex)
					WARN(ex.error());
				NOTE("The application is no more registered as a service");
			} else
				ERROR(ex.error());
			return EXIT_OK;
		}
		return main();
	} catch (exception& ex) {
		FATAL(ex.what());
		return EXIT_SOFTWARE;
	} catch (...) {
		FATAL("Unknown error");
		return EXIT_SOFTWARE;
	}
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
	string name, path;
	getString("application.baseName", name);
	getString("application.path", path);
	
	WinService service(name);
	if (_displayName.empty())
		service.registerService(ex,path);
	else
		service.registerService(ex, path, _displayName);
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
	string name;
	getString("application.baseName", name);
	
	WinService service(name);
	return service.unregisterService(ex);
}

void ServerApplication::defineOptions(Exception& ex,Options& options) {

	options.add(ex, "registerService", "r", "Register the application as a service.");

	options.add(ex, "unregisterService", "u", "Unregister the application as a service.");

	options.add(ex, "name", "n", "Specify a display name for the service (only with /registerService).")
		.argument("name")
		.handler([this](const string& value) { _displayName = value; });

	options.add(ex, "description", "d", "Specify a description for the service (only with /registerService).")
		.argument("text")
		.handler([this](const string& value) { _description = value; });

	options.add(ex, "startup", "s", "Specify the startup mode for the service (only with /registerService).")
		.argument("automatic|manual")
		.handler([this](const string& value) {_startup = Poco::icompare(value, 4, string("auto")) == 0 ? "auto" : "manual"; });

	Application::defineOptions(ex, options);
}


#else // _WIN32_WCE
void ServerApplication::waitForTerminationRequest() {
	_terminate.wait();
}

#endif // _WIN32_WCE
#elif defined(POCO_VXWORKS)
//
// VxWorks specific code
//
void ServerApplication::waitForTerminationRequest() {
	_terminate.wait();
}

#elif defined(POCO_OS_FAMILY_UNIX)


//
// Unix specific code
//
void ServerApplication::waitForTerminationRequest() {
	sigset_t sset;
	sigemptyset(&sset);
	if (!getenv("POCO_ENABLE_DEBUGGER"))
	{
		sigaddset(&sset, SIGINT);
	}
	sigaddset(&sset, SIGQUIT);
	sigaddset(&sset, SIGTERM);
	sigprocmask(SIG_BLOCK, &sset, NULL);
	int sig;
	sigwait(&sset, &sig);
}


int ServerApplication::run(int argc, char** argv) {
	try {
		bool runAsDaemon = isDaemon(argc, argv);
		if (runAsDaemon)
			beDaemon();
		if(!init(argc, argv))
			return EXIT_OK;

		if (runAsDaemon) {
			int rc = chdir("/");
			if (rc != 0)
				return EXIT_OSERR;
		}
		return main();
	} catch (exception& ex) {
		FATAL( ex.what());
		return EXIT_SOFTWARE;
	} catch (...) {
		FATAL("Unknown error");
		return EXIT_SOFTWARE;
	}
}


bool ServerApplication::isDaemon(int argc, char** argv) {
	string option1("--daemon");
	string option2("-d");
	string option3("/daemon");
	string option4("/d");
	for (int i = 1; i < argc; ++i) {
		if (stricmp(option1.c_str(),argv[i])==0 || stricmp(option2.c_str(),argv[i])==0 || stricmp(option3.c_str(),argv[i])==0 || stricmp(option4.c_str(),argv[i])==0)
			return true;
	}
	return false;
}


void ServerApplication::beDaemon() {
	pid_t pid;
	if ((pid = fork()) < 0)
		throw exception("Cannot fork daemon process");
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
		throw exception("Cannot attach stdin to /dev/null");
	FILE* fout = freopen("/dev/null", "r+", stdout);
	if (!fout)
		throw exception("Cannot attach stdout to /dev/null");
	FILE* ferr = freopen("/dev/null", "r+", stderr);
	if (!ferr)
		throw exception("Cannot attach stderr to /dev/null");

	setBool("application.runAsDaemon", true);
	_isInteractive=false;
}


void ServerApplication::defineOptions(OptionSet& options) {
	options.add("daemon", "d", "Run application as a daemon.")
		.handler([this](const string& value) { setBool("application.runAsDaemon", true) });
	
	options.add("pidfile", "p", "Write the process ID of the application to given file.")
		.argument("path")
		.handler(handlePidFile);

	Application::defineOptions(options);
}


void ServerApplication::handlePidFile(const string& value) {
	ofstream ostr(value.c_str());
	if (!ostr.good())
		throw Exception(Exception::APPLICATION,"Cannot write PID to file ",value);
	ostr << Poco::Process::id() << endl;
	Poco::TemporaryFile::registerForDeletion(value);
}


#elif defined(POCO_OS_FAMILY_VMS)


//
// VMS specific code
//
namespace
{
	static void handleSignal(int sig) {
		ServerApplication::terminate();
	}
}


void ServerApplication::waitForTerminationRequest() {
	struct sigaction handler;
	handler.sa_handler = handleSignal;
	handler.sa_flags   = 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);
	sigaction(SIGQUIT, &handler, NULL);                                       

	long ctrlY = LIB$M_CLI_CTRLY;
	unsigned short ioChan;
	$DESCRIPTOR(ttDsc, "TT:");

	lib$disable_ctrl(&ctrlY);
	sys$assign(&ttDsc, &ioChan, 0, 0);
	sys$qiow(0, ioChan, IO$_SETMODE | IO$M_CTRLYAST, 0, 0, 0, terminate, 0, 0, 0, 0, 0);
	sys$qiow(0, ioChan, IO$_SETMODE | IO$M_CTRLCAST, 0, 0, 0, terminate, 0, 0, 0, 0, 0);

	string evName("POCOTRM");
	String::Append(evName, Format<UInt64>("%08" I64_FMT "X", Poco::Process::id()));
	Poco::NamedEvent ev(evName);
	try
	{
		ev.wait();
    }
	catch (...)
	{
		// CTRL-C will cause an exception to be raised
	}
	sys$dassgn(ioChan);
	lib$enable_ctrl(&ctrlY);
}



#endif


} // namespace Mona