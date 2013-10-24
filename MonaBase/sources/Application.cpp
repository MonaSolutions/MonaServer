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
#include "Mona/Time.h"
#if defined(POCO_OS_FAMILY_WINDOWS)
#include "Poco/UnWindows.h"
#endif
#if defined(POCO_OS_FAMILY_UNIX) && !defined(POCO_VXWORKS)
#include "Poco/SignalHandler.h"
#endif


using namespace std;
using namespace Poco;

namespace Mona {

const char* LogPriorities[] = { "FATAL", "CRITIC", "ERROR", "WARN", "NOTE", "INFO", "DEBUG", "TRACE" };

Application::Application() : _logSizeByFile(1000000), _logRotation(10) {
	#if defined(POCO_OS_FAMILY_UNIX) && !defined(POCO_VXWORKS)
		_workingDirAtLaunch = Path::current();
		#if !defined(_DEBUG)
			Poco::SignalHandler::install();
		#endif
	#elif defined(_WIN32)
		DetectMemoryLeak();
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


bool Application::init(int argc, char* argv[]) {
	Path appPath;
	string command(argv[0]);
	getApplicationPath(command, appPath);
	setString("application.command", command);
	setString("application.path", appPath.toString());
	setString("application.name", appPath.getFileName());
	setString("application.baseName", appPath.getBaseName());
	setString("application.dir", appPath.parent().toString());
	

	// configurations
	string configPath(appPath.parent().toString());
	configPath.append(appPath.getBaseName());
	configPath.append(".ini");
	if (loadConfigurations(configPath)) {
		Path configPath(configPath);
		setString("application.configDir", configPath.parent().toString());
		setString("application.configPath", configPath.toString());
	}

	// logs
	Logs::SetLogger(*this);

	string logDir("logs"), logFileName("log");
	if (loadLogFiles(makeAbsolute(logDir), logFileName, _logSizeByFile, _logRotation)) {
		File(logDir).createDirectory();
		_logPath = logDir + "/" + logFileName;
		if (_logRotation > 0) {
			_logPath.append(".");
			_pLogFile.reset(new File(_logPath + "0"));
		} else
			_pLogFile.reset(new File(_logPath));
		_logStream.open(_pLogFile->path(), ios::in | ios::ate);
	}

	// options
	Exception ex;
	defineOptions(ex, _options);
	if (ex)
		throw exception(ex.error().c_str());
	if(!_options.process(ex,argc, argv, [this](Exception& ex,const string& name, const string& value){ setString("application." + name, value); }))
		throw exception(ex.error().c_str());

	if (hasArgument("help")) {
		displayHelp();
		return false;
	}
	return true;
}

bool Application::loadConfigurations(string& path) {
	Exception ex;
	if (Util::ReadIniFile(ex, path, *this)) {
		DEBUG("Impossible to load configuration file (", ex.error(), ")");
		return false;
	}
	return true;
}

bool Application::loadLogFiles(string& directory, string& fileName, UInt32& sizeByFile, UInt16& rotation) {
	getString("logs.directory", directory);
	getString("logs.name", fileName);
	int rotate;
	getNumber("logs.rotation", rotate);
	if (rotate < 0)
		throw exception("Invalid negative logs.rotation value");
	rotation = true;
	return true;
}

void Application::defineOptions(Exception& ex, Options& options) {

	options.add(ex, "log", "l", "Log level argument, must be beetween 0 and 8 : nothing, fatal, critic, error, warn, note, info, debug, trace. Default value is 6 (info), all logs until info level are displayed.")
		.argument("level");

	options.add(ex, "dump", "d", "Enables packet traces in logs. Optional arguments are 'intern' or 'all' respectively to displays just intern packet exchanged (between servers) or all packet process. If no argument is given, just outside packet process will be dumped.")
		.argument("intern|all", false)
		.handler([](const string& value) { Logs::SetDump(value == "all" ? Logs::DUMP_ALL : (value == "intern" ? Logs::DUMP_INTERN : Logs::DUMP_EXTERN)); });
	
	options.add(ex,"help", "h", "Displays help information about command-line usage.");
}

int Application::run(int argc, char* argv[]) {
	try {
		if (!init(argc, argv))
			return EXIT_OK;
		return main();
	} catch (exception& ex) {
		FATAL(ex.what());
		return EXIT_SOFTWARE;
	} catch (...) {
		FATAL("Unknown error");
		return EXIT_SOFTWARE;
	}
}

void Application::log(Thread::TID threadId, const string& threadName, Priority priority, const char *filePath, const string& shortFilePath, long line, const string& message) {
	if (isInteractive())
		Logger::log(threadId, threadName, priority, filePath, shortFilePath, line, message);
	if (!_logStream.good())
		return;
	lock_guard<mutex> lock(_logMutex);
	string stDate;
	_logStream << Time().toLocaleString("%d/%m %H:%M:%S.%c  ", stDate)
		<< LogPriorities[priority] << '\t' << threadName << '(' << threadId << ")\t"
		<< shortFilePath << '[' << line << "]  " << message << std::endl;
	_logStream.flush();
	manageLogFiles();
}

void Application::dump(const UInt8* data, UInt32 size) {
	if (isInteractive())
		Logger::dump(data, size);
	if (!_logStream.good())
		return;
	lock_guard<mutex> lock(_logMutex);
	_logStream.write((const char*)data, size);
	_logStream.flush();
	manageLogFiles();
}

void Application::manageLogFiles() {
	if (!_pLogFile || _logSizeByFile == 0)
		return;
	if (_pLogFile->getSize() > _logSizeByFile) {
		_logStream.close();
		if (_logRotation == 0) {
			File file(_logPath + "10");
			if (file.exists())
				file.remove();
		}
		int num = _logRotation;
		string path;
		if (num > 0)
			String::Append(path,num);
		File file(path);
		if(file.exists())
			file.remove();

		string stfile;
		// rotate
		while(--num>=0) {
			file = String::Format(stfile, _logPath, num);
			if(file.exists()) {
				String::Format(stfile, _logPath, num+1);
				file.renameTo(stfile);
			}
		}
		_logStream.open(_pLogFile->path(), ios::in | ios::ate);
	}
}


void Application::getApplicationPath(const string& command,Path& appPath) const {
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
	else
		throw exception("Impossible to determine the application file name");
#else
	appPath = command;
#endif
}

} // namespace Mona
