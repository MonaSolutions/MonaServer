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

#pragma once

#include "Mona/Mona.h"
#include "Mona/Logger.h"
#include "Mona/String.h"
#include "Mona/Util.h"
#include "Mona/MemoryReader.h"
#include "Mona/MemoryWriter.h"
#include "Poco/Path.h"

namespace Mona {

class Logs : virtual Static {
public:
	enum DumpMode {
		DUMP_NOTHING = 0,
		DUMP_EXTERN = 1,
		DUMP_INTERN = 2,
		DUMP_ALL = 3
	};

	static void			SetLogger(Logger& logger) { _PLogger = &logger; }
	static void			SetLevel(Poco::UInt8 level) { _Level = level; }
	static UInt8		GetLevel() { return _Level; }
	static void			SetDump(DumpMode mode) { _DumpMode = mode; }
	static DumpMode		GetDump() { return _DumpMode; }

	static void			Dump(const Poco::UInt8* data, Poco::UInt32 size, const char* header = NULL);
	static void			Dump(MemoryReader& packet, const char* header = NULL) { Dump(packet.current(), packet.available(), header); }
	static void			Dump(MemoryWriter& packet, const char* header = NULL) { Dump(packet.begin(), packet.length(), header); }
	static void			Dump(MemoryWriter& packet, Poco::UInt16 offset, const char* header = NULL) { Dump(packet.begin() + offset, packet.length() - offset, header); }
	
	template <typename ...Args>
	static void	Log(Logger::Priority prio, const char* file, long line, const Args&... args) {
		if (_Level < prio)
			return;
		Poco::Path path(file);
		string shortFile;
		if (path.depth() > 0)
			shortFile.assign(path.directory(path.depth() - 1) + "/");
		shortFile.append(path.getFileName());
		string message;
		String::Format(message, args ...);
		if (_PLogger)
			_PLogger->logHandler(Poco::Thread::currentTid(), Poco::Thread::current() ? Poco::Thread::current()->name() : "", prio, file, shortFile, line, message);
		else
			_DefaultLogger.logHandler(Poco::Thread::currentTid(), Poco::Thread::current() ? Poco::Thread::current()->name() : "", prio, file, shortFile, line, message);
	}

	template <typename ...Args>
	static void Dump(const UInt8* data, UInt32 size, const Args&... args) {
		vector<UInt8> out;
		string header;
		String::Format(header, args ...);
		Util::Dump(data, size, out, header);
		if (out.size() == 0)
			return;
		if (_PLogger)
			_PLogger->dumpHandler(&out[0], out.size());
		else
			_DefaultLogger.dumpHandler(&out[0], out.size());
	}

private:
	static Logger*		_PLogger;
	static DumpMode		_DumpMode;
	static UInt8		_Level;
	static Logger		_DefaultLogger;
};

#undef ERROR
#undef DEBUG
#undef TRACE
#define FATAL(...) { Logs::Log(Logger::PRIO_FATAL,__FILE__,__LINE__, __VA_ARGS__); }
#define CRITIC(...) { Logs::Log(Logger::PRIO_CRITIC,__FILE__,__LINE__, __VA_ARGS__); }
#define ERROR(...) { Logs::Log(Logger::PRIO_ERROR,__FILE__,__LINE__, __VA_ARGS__); }
#define WARN(...) { Logs::Log(Logger::PRIO_WARN,__FILE__,__LINE__, __VA_ARGS__); }
#define NOTE(...) { Logs::Log(Logger::PRIO_NOTE,__FILE__,__LINE__, __VA_ARGS__); }
#define INFO(...) { Logs::Log(Logger::PRIO_INFO,__FILE__,__LINE__, __VA_ARGS__); }
#define DEBUG(...) { Logs::Log(Logger::PRIO_DEBUG,__FILE__,__LINE__, __VA_ARGS__); }
#define TRACE(...) { Logs::Log(Logger::PRIO_TRACE,__FILE__,__LINE__, __VA_ARGS__); }

#define DUMP_INTERN(...) { if(Logs::GetDump()&Logs::DUMP_INTERN) {Logs::Dump(__VA_ARGS__);} }
#define DUMP(...) { if(Logs::GetDump()&Logs::DUMP_EXTERN) {Logs::Dump(__VA_ARGS__);} }


} // namespace Mona
