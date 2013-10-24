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

#include "Mona/Application.h"
#include "Mona/Logs.h"
#include "Mona/Exceptions.h"
#if defined(POCO_OS_FAMILY_WINDOWS)
#include "Poco/UnWindows.h"
#endif
#if defined(POCO_OS_FAMILY_UNIX) && !defined(POCO_VXWORKS)
#include "Poco/SignalHandler.h"
#endif


using namespace std;
using namespace Poco;

namespace Mona {

Application::Application() {
	#if defined(POCO_OS_FAMILY_UNIX) && !defined(POCO_VXWORKS)
		_workingDirAtLaunch = Path::current();
		#if !defined(_DEBUG)
			Poco::SignalHandler::install();
		#endif
	#endif
}

Application::~Application() {
}

string& Application::makeAbsolute(string& path) {
	string temp = std::move(path);
	getString("application.dir", path);
	path.append(temp);
	return path;
}


void Application::displayHelp() {
	HelpFormatter helpFormatter(options());
	string command;
	if (getString("application.command", command))
		helpFormatter.command = command;
	helpFormatter.usage = "[options]";
	helpFormatter.header = "options:";
	if (onHelp(helpFormatter))
		helpFormatter.flush(cout);
}


void Application::init(int argc, char* argv[]) {
	Path appPath;
	string command(argv[0]);
	Exception ex;
	getApplicationPath(ex,command, appPath);
	if (ex)
		throw exception(ex.error().c_str());
	setString("application.command", command);
	setString("application.path", appPath.toString());
	setString("application.name", appPath.getFileName());
	setString("application.baseName", appPath.getBaseName());
	setString("application.dir", appPath.parent().toString());
	setString("application.configDir", appPath.parent().toString());

	// options
	defineOptions(ex, _options);
	if (ex)
		throw exception(ex.error().c_str());
	if(!_options.process(ex,argc, argv, [this](Exception& ex,const string& name, const string& value){ setString("application." + name, value); }))
		throw exception(ex.error().c_str());
}

int Application::run(int argc, char* argv[]) {
	try {
		init(argc, argv);
		return main();
	} catch (exception& ex) {
		FATAL("%s", ex.what());
		return Application::EXIT_SOFTWARE;
	} catch (...) {
		FATAL("Unknown error");
		return Application::EXIT_SOFTWARE;
	}
}


bool Application::getApplicationPath(Exception& ex,const string& command,Path& appPath) const {
#if defined(POCO_OS_FAMILY_UNIX) && !defined(POCO_VXWORKS)
	if (command.find('/') != string::npos) {
		Path path(command);
		if (path.isAbsolute()) {
			appPath = path;
		} else {
			appPath = _workingDirAtLaunch;
			appPath.append(path);
		}
	} else {
		if (!Path::find(Environment::get("PATH"), command, appPath))
			appPath = Path(_workingDirAtLaunch, command);
		appPath.makeAbsolute();
	}
#elif defined(POCO_OS_FAMILY_WINDOWS)
	char path[1024];
	int n = GetModuleFileNameA(0, path, sizeof(path));
	if (n > 0)
		appPath = path;
	else {
		ex.set(Exception::APPLICATION, "Impossible to determine the application file name");
		return false;
	}	
#else
	appPath = command;
#endif
	return true;
}

} // namespace Mona
