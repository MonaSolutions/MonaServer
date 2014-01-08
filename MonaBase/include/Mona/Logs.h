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

#pragma once

#include "Mona/Mona.h"
#include "Mona/Logger.h"
#include "Mona/String.h"
#include "Mona/Util.h"

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
	static void			SetLevel(UInt8 level) { _Level = level; }
	static UInt8		GetLevel() { return _Level; }
	static void			SetDump(DumpMode mode) { _DumpMode = mode; }
	static DumpMode		GetDump() { return _DumpMode; }


	template <typename ...Args>
    static void	Log(Logger::Priority prio, const char* file, long line, Args&&... args) {
		if (_Level < prio)
			return;
		std::string shortFile(file);
		auto found = shortFile.find_last_of("\\/");
		if (found != std::string::npos) {
			found = shortFile.find_last_of("\\/", found - 1);
			if (found != std::string::npos)
				shortFile.erase(0,found+1);
		}
			
		std::string message;
		String::Format(message, args ...);
		if (_PLogger)
            _PLogger->log(std::this_thread::get_id(), Util::GetThreadName(std::this_thread::get_id()), prio, file, shortFile, line, message);
		else
            _DefaultLogger.log(std::this_thread::get_id(), Util::GetThreadName(std::this_thread::get_id()), prio, file, shortFile, line, message);
	}

	template <typename ...Args>
	static void Dump(const UInt8* data, UInt32 size, Args&&... args) {
		Buffer out;
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

private:
	static Logger*		_PLogger;
	static DumpMode		_DumpMode;
	static UInt8		_Level;
	static Logger		_DefaultLogger;
};

#undef ERROR
#undef DEBUG
#undef TRACE
#define FATAL(...) { Mona::Logs::Log(Mona::Logger::PRIO_FATAL,__FILE__,__LINE__, __VA_ARGS__); }
#define CRITIC(...) { Mona::Logs::Log(Mona::Logger::PRIO_CRITIC,__FILE__,__LINE__, __VA_ARGS__); }
#define ERROR(...) { Mona::Logs::Log(Mona::Logger::PRIO_ERROR,__FILE__,__LINE__, __VA_ARGS__); }
#define WARN(...) { Mona::Logs::Log(Mona::Logger::PRIO_WARN,__FILE__,__LINE__, __VA_ARGS__); }
#define NOTE(...) { Mona::Logs::Log(Mona::Logger::PRIO_NOTE,__FILE__,__LINE__, __VA_ARGS__); }
#define INFO(...) { Mona::Logs::Log(Mona::Logger::PRIO_INFO,__FILE__,__LINE__, __VA_ARGS__); }
#define DEBUG(...) { Mona::Logs::Log(Mona::Logger::PRIO_DEBUG,__FILE__,__LINE__, __VA_ARGS__); }
#define TRACE(...) { Mona::Logs::Log(Mona::Logger::PRIO_TRACE,__FILE__,__LINE__, __VA_ARGS__); }

#define DUMP_INTERN(...) { if(Mona::Logs::GetDump()&Mona::Logs::DUMP_INTERN) {Mona::Logs::Dump(__VA_ARGS__);} }
#define DUMP(...) { if(Mona::Logs::GetDump()&Mona::Logs::DUMP_EXTERN) {Mona::Logs::Dump(__VA_ARGS__);} }


} // namespace Mona
