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
#include "Mona/MemoryReader.h"
#include "Mona/MemoryWriter.h"

namespace Mona {

class Logs
{
public:
	enum DumpMode {
		NOTHING = 0,
		EXTERN,
		INTERN,
		ALL
	};

	static void			SetLogger(Logger& logger) { _PLogger = &logger; }
	static void			SetLevel(Poco::UInt8 level) { _Level = level; }
	static void			SetDump(DumpMode mode) { _DumpMode = mode; }

	static DumpMode				GetDump() { return _DumpMode; }
	static Logger*				GetLogger() { return _PLogger; }
	static Poco::UInt8			GetLevel() { return _Level; }
	static void					Dump(const Poco::UInt8* data, Poco::UInt32 size, const char* header = NULL);
	static void					Dump(MemoryReader& packet, const char* header = NULL) { Dump(packet.current(), packet.available(), header); }
	static void					Dump(MemoryWriter& packet, const char* header = NULL) { Dump(packet.begin(), packet.length(), header); }
	static void					Dump(MemoryWriter& packet, Poco::UInt16 offset, const char* header = NULL) { Dump(packet.begin() + offset, packet.length() - offset, header); }
	
	template <typename ...Args>
	static void					Log(Logger::Priority prio, const char* file, long line, const Args&... args) {

		if (Mona::Logs::GetLogger()) {
			if (Mona::Logs::GetLevel() >= prio) {
			
				String::Format(_logmsg, args ...);
				Mona::Logs::GetLogger()->logHandler(Poco::Thread::currentTid(), Poco::Thread::current() ? Poco::Thread::current()->name() : "", prio, file, line, _logmsg.c_str());
			}
		}
		else if (Mona::Logs::GetLevel() >= Mona::Logger::PRIO_ERROR) {
		
			String::Format(_logmsg, args ...);
			throw(_logmsg);
			// TODO delete this part?
		}
	}

private:
	Logs() {}
	virtual ~Logs() {}
	
	static Logger*		_PLogger;
	static DumpMode		_DumpMode;
	static Poco::UInt8  _Level;
	static std::string	_logmsg;
};

// Empecher le traitement des chaines si de toute façon le log n'a aucun Logger!
// Ou si le level est plus détaillé que le loglevel
#define LOG(PRIO,FILE,LINE, ...) { Mona::Logs::Log(PRIO, FILE, LINE, ## __VA_ARGS__); }

#undef ERROR
#undef DEBUG
#undef TRACE
#define FATAL(...) LOG(Mona::Logger::PRIO_FATAL,__FILE__,__LINE__, ## __VA_ARGS__)
#define CRITIC(...) LOG(Mona::Logger::PRIO_CRITIC,__FILE__,__LINE__, ## __VA_ARGS__)
#define ERROR(...) LOG(Mona::Logger::PRIO_ERROR,__FILE__,__LINE__, ## __VA_ARGS__)
#define WARN(...) LOG(Mona::Logger::PRIO_WARN,__FILE__,__LINE__, ## __VA_ARGS__)
#define NOTE(...) LOG(Mona::Logger::PRIO_NOTE,__FILE__,__LINE__, ## __VA_ARGS__)
#define INFO(...) LOG(Mona::Logger::PRIO_INFO,__FILE__,__LINE__, ## __VA_ARGS__)
#define DEBUG(...) LOG(Mona::Logger::PRIO_DEBUG,__FILE__,__LINE__, ## __VA_ARGS__)
#define TRACE(...) LOG(Mona::Logger::PRIO_TRACE,__FILE__,__LINE__, ## __VA_ARGS__)

#define DUMP_INTERN(...) { if(Mona::Logs::GetLogger() && (Mona::Logs::GetDump()&Mona::Logs::INTERN)) {Mona::Logs::Dump(__VA_ARGS__);} }
#define DUMP(...) { if(Mona::Logs::GetLogger() && (Mona::Logs::GetDump()&Mona::Logs::EXTERN)) {Mona::Logs::Dump(__VA_ARGS__);} }


} // namespace Mona
