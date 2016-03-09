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
	helpFormatter.command = _file.path();
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
	string configPath;
	if (loadConfigurations(String::Format(configPath,_file.parent(),_file.baseName(),".ini"))) {
		setString("application.configPath", configPath);
		setString("application.configDir", FileSystem::GetParent(configPath));
	}

	// logs
	Logs::SetLogger(*this);

	string logDir(_file.parent());
	logDir.append("logs");
	string logFileName("log");

	Exception ex;
	if (loadLogFiles(logDir, logFileName, _logSizeByFile, _logRotation)) {
		bool success;
		EXCEPTION_TO_LOG(success=FileSystem::CreateDirectory(ex, logDir),file().baseName()," log system")
		if (success) {
			_logPath.assign(FileSystem::MakeFolder(logDir)).append(logFileName);
			if (_logRotation > 0) {
				_logPath += '.';
				_logStream.open(_logPath + '0', ios::out | ios::binary | ios::app);
			}  else
				_logStream.open(_logPath, ios::out | ios::binary | ios::app);
		}
	}

	// options
	defineOptions(ex, _options);
	if (ex)
        FATAL_ERROR(ex.error());

	if (!_options.process(ex, argc, argv, [this](const string& name, const string& value) { setString("arguments." + name, value); }))
        FATAL_ERROR("Arguments, ",ex.error()," use 'help'")
	else if (ex)
		WARN("Arguments, ",ex.error()," use 'help'")

	if (hasArgument("help")) {
		displayHelp();
		return false;
	}
	return true;
}

bool Application::loadConfigurations(string& path) {
	if (Util::ReadIniFile(path, *this))
		DEBUG("Load configuration file ",path)
	else
		DEBUG("Impossible to load configuration file ",path)
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
		.handler([this](Exception& ex, const string& value) {
#if defined(_DEBUG)
		Logs::SetLevel(String::ToNumber<UInt8>(ex, Logger::LEVEL_DEBUG, value));
#else
		Logs::SetLevel(String::ToNumber<UInt8>(ex, Logger::LEVEL_INFO, value));
#endif
		return true;
	});

	options.add(ex, "dump", "d", "Enables packet traces in logs. Optional argument is a string filter to dump just packet which matchs this expression. If no argument is given, all the dumpable packet are gotten.")
		.argument("filter", false)
		.handler([this](Exception& ex, const string& value) { Logs::SetDump(value.c_str()); return true; });
	
	options.add(ex, "dumplimit", "dl", "If dump is activated this option set the limit of dump messages. Argument is an unsigned integer defining the limit of bytes to show. By default there is not limit.")
		.argument("limit", true)
		.handler([this](Exception& ex, const string& value) { Logs::SetDumpLimit(String::ToNumber<Int32>(ex,-1, value)); return true; });

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

void Application::log(THREAD_ID threadId, Level level, const char *filePath, string& shortFilePath, long line, string& message) {
	if (isInteractive())
		Logger::log(threadId, level, filePath, shortFilePath, line, message);
	if (!_logStream.good()) // outside _logMutex because must stay constant, otherwise it means that write operation has raised an exception (a log folder/file retablishment is not supported today)
		return;
	lock_guard<mutex> lock(_logMutex);
	string date;
	string threadName;
	_logStream << Date().toString("%d/%m %H:%M:%S.%c  ", date)
		<< LogLevels[level-1] << '\t' << Util::GetThreadName(threadId,threadName) << '(' << threadId << ")\t"
		<< shortFilePath << '[' << line << "]  " << message << std::endl;
	_logStream.flush();
	manageLogFiles();
}

void Application::dump(const string& header, const UInt8* data, UInt32 size) {
	if (isInteractive())
		Logger::dump(header, data, size);
	if (!_logStream.good()) // outside _logMutex because must stay constant, otherwise it means that write operation has raised an exception (a log folder/file retablishment is not supported today)
		return;
	lock_guard<mutex> lock(_logMutex);
	string date;
	_logStream << Date().toString("%d/%m %H:%M:%S.%c  ", date);
	_logStream.write(header.data(), header.size()).put('\n');
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
		EXCEPTION_TO_LOG(FileSystem::Delete(ex,path),"Log manager")
		// rotate
		string newPath;
		while(--num>=0)
			FileSystem::Rename(String::Format(path, _logPath, num), String::Format(newPath, _logPath, num + 1));
		if (_logRotation>0)
			_logStream.open(String::Format(newPath, _logPath, '0'), ios::out | ios::binary | ios::app);
		else
			_logStream.open(_logPath, ios::out | ios::binary | ios::app);
	}
}


// TODO test linux/windows (service too)
void Application::initApplicationPaths(const char* command) {

	string path(command);

#if defined(_WIN32)
	FileSystem::GetCurrentApp(path);
#else
	if(!FileSystem::GetCurrentApp(path)) {

		if (path.find('/') != string::npos) {
			if (!FileSystem::IsAbsolute(path))
				path.insert(0,FileSystem::GetCurrentDir(""));
		} else {
			// just file name!
			string paths;
			if (!Util::Environment().getString("PATH", paths) || !FileSystem::ResolveFileWithPaths(paths, path))
				path.insert(0,FileSystem::GetCurrentDir(""));
		}
	}
#endif

	_file.setPath(path);

	setString("application.command", path);
	setString("application.path", path);
	setString("application.name", _file.name());
	setString("application.baseName", _file.baseName());
	setString("application.dir", _file.parent());
}

} // namespace Mona
