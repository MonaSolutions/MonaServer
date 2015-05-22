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
	static void			SetLogger(Logger& logger) { std::lock_guard<std::mutex> lock(_Mutex); _PLogger = &logger; }

	static void			SetLevel(UInt8 level) { std::lock_guard<std::mutex> lock(_Mutex); _Level = level; }
	static UInt8		GetLevel() { return _Level; }

	static void			SetDumpLimit(Int32 limit) { std::lock_guard<std::mutex> lock(_Mutex); _DumpLimit = limit; }
	static void			SetDump(const char* name); // if null, no dump, otherwise dump name, and if name is empty everything is dumped
	static bool			IsDumping() { return _PDump ? true : false; }

	template <typename ...Args>
    static void	Log(Logger::Level level, const char* file, long line, Args&&... args) {
		std::lock_guard<std::mutex> lock(_Mutex);
		if (_Level < level)
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
            _PLogger->log(Util::CurrentThreadId(), level, file, shortFile, line, message);
		else
            _DefaultLogger.log(Util::CurrentThreadId(), level, file, shortFile, line, message);
	}

	template <typename ...Args>
	static void Dump(const std::string& name, const UInt8* data, UInt32 size, Args&&... args) {
		std::shared_ptr<std::string> pDump(_PDump);
		if (!pDump || (!pDump->empty() && String::ICompare(*pDump,name)!=0))
			return;
		std::string header(name);
		Dump(String::Append(header,"=> ", args ...), data, size);
	}

	template <typename ...Args>
	static void Dump(const char* name, const UInt8* data, UInt32 size, Args&&... args) {
		std::shared_ptr<std::string> pDump(_PDump);
		if (!pDump || (!pDump->empty() && String::ICompare(*pDump,name)!=0))
			return;
		std::string header(name);
		Dump(String::Append(header,"=> ", args ...), data, size);
	}

#if defined(_DEBUG)
	// To dump easly during debugging => no name filter = always displaid even if no dump argument
	static void Dump(const UInt8* data, UInt32 size) { Dump(String::Empty, data, size); }
#endif

private:

	static void Dump(const std::string& header, const UInt8* data, UInt32 size);


	static std::mutex	_Mutex;

	static UInt8		_Level;
	static Logger*		_PLogger;
	static Logger		_DefaultLogger;

	static std::shared_ptr<std::string>	 _PDump; // NULL means no dump, empty() means all dump, otherwise is a dump filter
	static Int32						 _DumpLimit; // -1 means no limit
};

#undef ERROR
#undef DEBUG
#undef TRACE

#define LOG_BUFFER	__buffer
#define LOG(LEVEL,FILE,LINE,...) { if(Mona::Logs::GetLevel()>=LEVEL) { std::string __buffer; Mona::Logs::Log(LEVEL,FILE,LINE, __VA_ARGS__); } }

#define FATAL(...)	LOG(Mona::Logger::LEVEL_FATAL,__FILE__,__LINE__, __VA_ARGS__)
#define CRITIC(...) LOG(Mona::Logger::LEVEL_CRITIC,__FILE__,__LINE__, __VA_ARGS__)
#define ERROR(...)	LOG(Mona::Logger::LEVEL_ERROR,__FILE__,__LINE__, __VA_ARGS__)
#define WARN(...)	LOG(Mona::Logger::LEVEL_WARN,__FILE__,__LINE__, __VA_ARGS__)
#define NOTE(...)	LOG(Mona::Logger::LEVEL_NOTE,__FILE__,__LINE__, __VA_ARGS__)
#define INFO(...)	LOG(Mona::Logger::LEVEL_INFO,__FILE__,__LINE__, __VA_ARGS__)
#define DEBUG(...)	LOG(Mona::Logger::LEVEL_DEBUG,__FILE__,__LINE__, __VA_ARGS__)
#define TRACE(...)	LOG(Mona::Logger::LEVEL_TRACE,__FILE__,__LINE__, __VA_ARGS__)

#define DUMP(NAME,...) { if(Mona::Logs::IsDumping()) Mona::Logs::Dump(NAME,__VA_ARGS__); }


} // namespace Mona
