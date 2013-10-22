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

#if defined(POCO_OS_FAMILY_WINDOWS)
	#define FATAL_COLOR 12
	#define CRITIC_COLOR 12
	#define ERROR_COLOR 13
	#define WARN_COLOR 14
	#define NOTE_COLOR 10
	#define INFO_COLOR 15
	#define DEBUG_COLOR 7
	#define TRACE_COLOR 8
	#define SET_CONSOLE_TEXT_COLOR(color) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color)
#else
	#define FATAL_COLOR "\033[01;31m"
	#define	CRITIC_COLOR "\033[01;31m"
	#define ERROR_COLOR "\033[01;35m"
	#define WARN_COLOR "\033[01;33m"	
	#define NOTE_COLOR "\033[01;32m"
	#define INFO_COLOR "\033[01;37m"
	#define DEBUG_COLOR "\033[0m"
	#define TRACE_COLOR "\033[01;30m"
	#define SET_CONSOLE_TEXT_COLOR(color) fprintf(stdout,"%s",color)
#endif


#define LOG_SIZE 1000000

using namespace Mona;
using namespace Poco;
using namespace Poco::Net;
using namespace std;


const char* LogPriorities[] = { "FATAL","CRITIC" ,"ERROR","WARN","NOTE","INFO","DEBUG","TRACE" };
#if defined(POCO_OS_FAMILY_WINDOWS)
int			LogColors[] = { FATAL_COLOR,CRITIC_COLOR ,ERROR_COLOR,WARN_COLOR,NOTE_COLOR,INFO_COLOR,DEBUG_COLOR,TRACE_COLOR };
#else
const char* LogColors[] = { FATAL_COLOR,CRITIC_COLOR ,ERROR_COLOR,WARN_COLOR,NOTE_COLOR,INFO_COLOR,DEBUG_COLOR,TRACE_COLOR };
#endif


class ServerMona : public Mona::ServerApplication, private Mona::Logger, private ApplicationKiller  {
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

	void defineOptions(Mona::Options& options) {
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
		Mona::Logs::SetLogger(*this);

		// options 

		Mona::Exception ex;
		options.add(ex,
			Mona::Option("log", "l", "Log level argument, must be beetween 0 and 8 : nothing, fatal, critic, error, warn, note, info, debug, trace. Default value is 6 (info), all logs until info level are displayed.")
			.argument("level")
			 // TODO changer pour directement appeler l'aide??
		);
		if (ex)
			INFO(ex.error());

		options.add(ex, 
			Mona::Option("dump", "d", "Enables packet traces in logs. Optional arguments are 'intern' or 'all' respectively to displays just intern packet exchanged (between servers) or all packet process. If no argument is given, just outside packet process will be dumped.")
			.argument("intern|all", false)
			.handler([](const string& value) { Mona::Logs::SetDump(value == "all" ? Mona::Logs::ALL : (value == "intern" ? Mona::Logs::INTERN : Mona::Logs::EXTERN)); })
		);
		if (ex)
			INFO(ex.error());

		ServerApplication::defineOptions(options);
	}

	void dumpHandler(const Mona::UInt8* data,Mona::UInt32 size) {
		ScopedLock<FastMutex> lock(_logMutex);
		if(_isInteractive)
			cout.write((const char*)data,size);
		_logStream.write((const char*)data,size);
		_logStream.flush();
		manageLogFile();
	}

	void logHandler(Thread::TID threadId,const std::string& threadName,Priority priority,const char *filePath,long line, const char *text) {
		ScopedLock<FastMutex> lock(_logMutex);

		Path path(filePath);
		string file,shortName;
		if(path.getExtension() == "lua") {
			if(path.depth()>0)
				file.assign(path.directory(path.depth()-1) + "/");
			shortName.assign(file + path.getFileName());
		} else
			shortName.assign(path.getBaseName());

		priority = (Priority)(priority-1);
		if(_isInteractive) {
			SET_CONSOLE_TEXT_COLOR(LogColors[priority]);
			printf("%s[%ld] %s\n",shortName.c_str(),line,text);
			SET_CONSOLE_TEXT_COLOR(LogColors[6]);
			cout.flush();
		}

		string stDate;
		Time().toLocaleString(stDate, "%d/%m %H:%M:%S.%c  ");
		_logStream << stDate 
				<< LogPriorities[priority] << '\t' << threadName << '(' << threadId << ")\t"
				<< (file + path.getFileName()) << '[' << line << "]  " << text << std::endl;
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
		Mona::Exception ex;
		Mona::Util::ReadIniFile(ex, makeAbsolute(file), *this);
		if (ex)
			DEBUG("Impossible to find file MonaServer.ini (", ex.error(), ")");
		
		
#if defined(POCO_OS_FAMILY_UNIX)
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
