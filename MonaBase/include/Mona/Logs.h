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


	template <typename ...Args>
	static void	Log(Logger::Priority prio, const char* file, long line, const Args&... args) {
		if (_Level < prio)
			return;
		Poco::Path path(file);
		std::string shortFile;
		if (path.depth() > 0)
			shortFile.assign(path.directory(path.depth() - 1) + "/");
		shortFile.append(path.getFileName());
		std::string message;
		String::Format(message, args ...);
		if (_PLogger)
			_PLogger->log(Poco::Thread::currentTid(), Poco::Thread::current() ? Poco::Thread::current()->name() : "", prio, file, shortFile, line, message);
		else
			_DefaultLogger.log(Poco::Thread::currentTid(), Poco::Thread::current() ? Poco::Thread::current()->name() : "", prio, file, shortFile, line, message);
	}

	template <typename ...Args>
	static void Dump(const UInt8* data, UInt32 size, const Args&... args) {
		Buffer<UInt8> out;
		std::string header;
		String::Format(header, args ...);
		Util::Dump(data, size, out, header);
		if (out.size() == 0)
			return;
		if (_PLogger)
			_PLogger->dump(&out[0], out.size());
		else
			_DefaultLogger.dump(&out[0], out.size());
	}
	template <typename ...Args>
	static void	Dump(MemoryReader& packet, const Args&... args) { Dump(packet.current(), packet.available(), args ...);	}
	template <typename ...Args>
	static void	Dump(MemoryWriter& packet, const Args&... args) { Dump(packet.begin(), packet.length(), args ...); }
	template <typename ...Args>
	static void	Dump(MemoryWriter& packet, UInt16 offset, const Args&... args) { Dump(packet.begin() + offset, packet.length() - offset, args ...); }


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
