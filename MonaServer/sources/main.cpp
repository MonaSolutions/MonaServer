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

#include "MonaServer.h"
#include "ApplicationKiller.h"
#include "Mona/Logs.h"
#include "Mona/ServerApplication.h"
#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Mona/Time.h"

#if defined(POCO_OS_FAMILY_UNIX)
#include <signal.h>
#endif

#define LOG_SIZE 1000000

using namespace Mona;
using namespace Poco;
using namespace std;

const char* LogPriorities[] = { "FATAL", "CRITIC", "ERROR", "WARN", "NOTE", "INFO", "DEBUG", "TRACE" };

class ServerMona : public ServerApplication, private Logger, private ApplicationKiller  {
public:
	ServerMona() : _isInteractive(true), _pLogFile(NULL) {
	}
	
	~ServerMona() {
		if(_pLogFile)
			delete _pLogFile;
	}

private:

	void kill() {
		terminate();
	}

	void defineOptions(Options& options) {
		// logs
		_isInteractive = isInteractive();
		string logDir("logs");
		makeAbsolute(logDir);
		getString("logs.directory", logDir);

		File(logDir).createDirectory();
		string file("log");
		getString("logs.name", file);
		_logPath = logDir + "/" + file + ".";
		_pLogFile = new File(_logPath + "0");
		_logStream.open(_pLogFile->path(), ios::in | ios::ate);
		Logs::SetLogger(*this);

		// options 

		Exception ex;
		options.add(ex,
			Option("log", "l", "Log level argument, must be beetween 0 and 8 : nothing, fatal, critic, error, warn, note, info, debug, trace. Default value is 6 (info), all logs until info level are displayed.")
			.argument("level")
			 // TODO changer pour directement appeler l'aide??
			 // TODO assign the log, if prio = 0 what's happen?
		);
		if (ex)
			INFO(ex.error());

		options.add(ex, 
			Option("dump", "d", "Enables packet traces in logs. Optional arguments are 'intern' or 'all' respectively to displays just intern packet exchanged (between servers) or all packet process. If no argument is given, just outside packet process will be dumped.")
			.argument("intern|all", false)
			.handler([](const string& value) { Logs::SetDump(value == "all" ? Logs::DUMP_ALL : (value == "intern" ? Logs::DUMP_INTERN : Logs::DUMP_EXTERN)); })
		);
		if (ex)
			INFO(ex.error());

		ServerApplication::defineOptions(options);
	}

	void dumpHandler(const UInt8* data,UInt32 size) {
		if (_isInteractive)
			Logger::dumpHandler(data, size);
		ScopedLock<FastMutex> lock(_logMutex);
		_logStream.write((const char*)data,size);
		_logStream.flush();
		manageLogFile();
	}

	void logHandler(Thread::TID threadId, const std::string& threadName, Priority priority, const char *filePath, const string& shortFilePath, long line, const string& message) {
		if (_isInteractive)
			Logger::logHandler(threadId, threadName, priority, filePath, shortFilePath, line, message);
		
		ScopedLock<FastMutex> lock(_logMutex);
		string stDate;
		Time().toLocaleString(stDate, "%d/%m %H:%M:%S.%c  ");
		_logStream << stDate 
				<< LogPriorities[priority] << '\t' << threadName << '(' << threadId << ")\t"
				<< shortFilePath << '[' << line << "]  " << message << std::endl;
		_logStream.flush();
		manageLogFile();
	}

	void manageLogFile() {
		if(_pLogFile->getSize()>LOG_SIZE) {
			_logStream.close();
			int num = 10;
			File file(_logPath+"10");
			if(file.exists())
				file.remove();

			string stfile;
			while(--num>=0) {
				file = String::Format(stfile, _logPath, num);
				if(file.exists()) {
					String::Format(stfile, _logPath, num+1);
					file.renameTo(stfile);
				}
			}
			_logStream.open(_pLogFile->path(),ios::in | ios::ate);
		}	
	}


///// MAIN
	int main() {
		// configs MonaServer.ini
		string file("MonaServer.ini");
		Exception ex;
		Util::ReadIniFile(ex, makeAbsolute(file), *this);
		if (ex)
			DEBUG("Impossible to find file MonaServer.ini (", ex.error(), ")");
		
		
#if defined(POCO_OS_FAMILY_UNIX) // TODO remonter d'un niveau?
		sigset_t sset;
		sigemptyset(&sset);
		if (!getenv("POCO_ENABLE_DEBUGGER"))
			sigaddset(&sset, SIGINT);
		sigaddset(&sset, SIGQUIT);
		sigaddset(&sset, SIGTERM);
		sigprocmask(SIG_BLOCK, &sset, NULL);
#endif

		// starts the server
		int bufferSize(0), threads(0), serversPort(0);
		getNumber("bufferSize", bufferSize);
		getNumber("threads", threads);
		string serversTargets;
		getNumber("servers.port", serversPort);
		getString("servers.targets", serversTargets);
		MonaServer server(*this, bufferSize, threads, serversPort, serversTargets);
		server.start(*this);

		// wait for CTRL-C or kill
#if defined(POCO_OS_FAMILY_UNIX)
		int sig;
		sigwait(&sset, &sig);		
#else
		waitForTerminationRequest();
#endif

		// Stop the server
		server.stop();
		return Application::EXIT_OK;
	}
	
	bool					_isInteractive;
	string					_logPath;
	File*					_pLogFile;
	FileOutputStream		_logStream;
	FastMutex				_logMutex;
};

int main(int argc, char* argv[]) {
	DetectMemoryLeak();
	return ServerMona().run(argc, argv);
}
