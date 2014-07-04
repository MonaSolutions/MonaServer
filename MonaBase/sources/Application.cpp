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

#include "Mona/Application.h"
#include "Mona/Exceptions.h"
#include "Mona/Date.h"
#if !defined(_WIN32)
    #include <signal.h>
#else
	#include <windows.h>
#endif
#include "Mona/FileSystem.h"
#include "Mona/Logs.h"
#include <sstream>
#include <iostream>

using namespace std;


namespace Mona {

static const char* LogLevels[] = { "FATAL", "CRITIC", "ERROR", "WARN", "NOTE", "INFO", "DEBUG", "TRACE" };

Application::Application() : _logSizeByFile(1000000), _logRotation(10) {
#if defined(_DEBUG)
#if defined(_WIN32)
	DetectMemoryLeak();
#else
	struct sigaction sa;
	sa.sa_handler = HandleSignal;
	sa.sa_flags   = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGILL, &sa, 0);
	sigaction(SIGBUS, &sa, 0);
	sigaction(SIGSEGV, &sa, 0);
	sigaction(SIGSYS, &sa, 0);
#endif
#endif
}

#if defined(_OS_UNIX)
void Application::HandleSignal(int sig) {
	switch (sig) {
		case SIGILL:
            FATAL_ERROR("Illegal instruction");
		case SIGBUS:
            FATAL_ERROR("Bus error");
		case SIGSEGV:
            FATAL_ERROR("Segmentation violation");
		case SIGSYS:
            FATAL_ERROR("Invalid system call");
		default:
            FATAL_ERROR("Error code ",sig)
	}
}
#endif

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


bool Application::init(int argc, const char* argv[]) {

	// set main thread name
	Util::SetCurrentThreadName("Main");

	initApplicationPaths(argv[0]);
	

	// configurations
	string dir;
	getString("application.dir", dir);
	string  name("configs");
	getString("application.baseName", name);
	string configPath(dir);
	configPath.append(name);
	configPath.append(".ini");
	if (loadConfigurations(configPath)) {
		setString("application.configPath", configPath);
		vector<string> values;
		FileSystem::Unpack(configPath, values);
		if (!values.empty())
			values.resize(values.size() - 1);
		FileSystem::Pack(values, configPath);
		setString("application.configDir", configPath);
		
	}

	// logs
	Logs::SetLogger(*this);

	string logDir(dir);
	logDir.append("logs");
	string logFileName("log");

	if (loadLogFiles(logDir, logFileName, _logSizeByFile, _logRotation)) {
		FileSystem::CreateDirectory(logDir);
		_logPath = logDir + "/" + logFileName;
		if (_logRotation > 0) {
			_logPath.append(".");
			_logStream.open(_logPath + "0", ios::out | ios::binary | ios::app);
		}  else
			_logStream.open(_logPath, ios::out | ios::binary | ios::app);
	}

	// options
	Exception ex;
	defineOptions(ex, _options);
	if (ex)
        FATAL_ERROR(ex.error());
	if(!_options.process(ex,argc, argv, [this](Exception& ex,const string& name, const string& value){ setString("application." + name, value); }))
        FATAL_ERROR(ex.error());

	if (hasArgument("help")) {
		displayHelp();
		return false;
	}
	return true;
}

bool Application::loadConfigurations(string& path) {
	Exception ex;
	if (Util::ReadIniFile(ex, path, *this))
		return true;
	DEBUG("Impossible to load configuration file (", ex.error(), ")");
	return false;
}

bool Application::loadLogFiles(string& directory, string& fileName, UInt32& sizeByFile, UInt16& rotation) {
	getString("logs.directory", directory);
	getString("logs.name", fileName);
	getNumber("logs.rotation", rotation);
	getNumber("logs.maxSize", sizeByFile);
	return true;
}

void Application::defineOptions(Exception& ex, Options& options) {

	options.add(ex, "log", "l", "Log level argument, must be beetween 0 and 8 : nothing, fatal, critic, error, warn, note, info, debug, trace. Default value is 6 (info), all logs until info level are displayed.")
		.argument("level")
		.handler([this](Exception& ex, const string& value) { Exception exWarn;
#if defined(_DEBUG)
		UInt8 level(Logger::LEVEL_DEBUG);
#else
		UInt8 level(Logger::LEVEL_INFO);
#endif
		Logs::SetLevel(String::ToNumber<UInt8>(exWarn, value,level)); if (exWarn) WARN("Bad level ",value," has to be a numeric value between 0 and 8 (see help)") });

	options.add(ex, "dump", "d", "Enables packet traces in logs. Optional arguments are 'intern' or 'all' respectively to displays just intern packet exchanged (between servers) or all packet process. If no argument is given, just outside packet process will be dumped.")
		.argument("intern|all", false)
		.handler([this](Exception& ex, const string& value) { Logs::SetDump(value == "all" ? Logs::DUMP_ALL : (value == "intern" ? Logs::DUMP_INTERN : Logs::DUMP_EXTERN)); });
	
	options.add(ex,"help", "h", "Displays help information about command-line usage.");
}

int Application::run(int argc, const char* argv[]) {
#if !defined(_DEBUG)
	try {
#endif
		if (!init(argc, argv))
			return EXIT_OK;
		return main();
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

void Application::log(THREAD_ID threadId, const string& threadName, Level level, const char *filePath, string& shortFilePath, long line, string& message) {
	if (isInteractive())
		Logger::log(threadId, threadName, level, filePath, shortFilePath, line, message);
	lock_guard<mutex> lock(_logMutex);
	if (!_logStream.good())
		return;
	string date;
	_logStream << Date().toString("%d/%m %H:%M:%S.%c  ", date)
		<< LogLevels[level-1] << '\t' << threadName << '(' << threadId << ")\t"
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
	if (_logSizeByFile == 0)
		return;
	if (_logStream.tellp() > _logSizeByFile) {
		_logStream.close();
		int num = _logRotation;
		string path(_logPath);
		if (num > 0)
			String::Append(path,num);
		Exception ex;
		EXCEPTION_TO_LOG(FileSystem::Remove(ex,path),"Log manager")
		// rotate
		string newPath;
		while(--num>=0)
			FileSystem::Rename(String::Format(path, _logPath, num), String::Format(newPath, _logPath, num + 1));
		if (_logRotation>0)
			_logStream.open(_logPath+"0", ios::out | ios::binary | ios::app);
		else
			_logStream.open(_logPath, ios::out | ios::binary | ios::app);
	}
}


// TODO test linux/windows (service too)
void Application::initApplicationPaths(const char* command) {
	if (hasKey("application.command"))
		return; // already done!

	string path(command);

#if defined(_WIN32)
	FileSystem::GetCurrentApplication(path);
#else
	if(!FileSystem::GetCurrentApplication(path)) {
		if (path.find('/') != string::npos) {
			if (!FileSystem::IsAbsolute(path)) {
				string temp = move(path);
				FileSystem::GetCurrent(path);
				path.append(temp);
			}
		} else {
			string paths;
			if (!Util::Environment().getString("PATH", paths) || !FileSystem::ResolveFileWithPaths(paths, path)) {
				string temp = move(path);
				FileSystem::GetCurrent(path);
				path.append(temp);
			}
		}
	}
#endif

	setString("application.command", path);

	vector<string> values;
	FileSystem::Unpack(path, values);

	setString("application.path", path);
	setString("application.name", values.empty() ? String::Empty : values.back());
	string baseName;
	setString("application.baseName", values.empty() ? String::Empty : FileSystem::GetBaseName(values.back(),baseName));

	if (!values.empty())
		values.resize(values.size() - 1);
	FileSystem::Pack(values, path);
	setString("application.dir", FileSystem::MakeDirectory(path));
}

} // namespace Mona
