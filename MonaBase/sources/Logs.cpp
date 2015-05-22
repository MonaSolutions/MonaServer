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

#include "Mona/Logs.h"
#include "Mona/Util.h"

using namespace std;

namespace Mona {

mutex							Logs::_Mutex;
Logger*							Logs::_PLogger(NULL);
std::shared_ptr<std::string> 	Logs::_PDump;
Int32							Logs::_DumpLimit(-1);
#if defined(_DEBUG)
UInt8							Logs::_Level(Logger::LEVEL_DEBUG); // default log level
#else
UInt8							Logs::_Level(Logger::LEVEL_INFO); // default log level
#endif
Logger							Logs::_DefaultLogger;


void Logs::SetDump(const char* name) {
	std::lock_guard<std::mutex> lock(_Mutex);
	if (!name)
		_PDump.reset();
	else
		_PDump.reset(new string(name));
}

void Logs::Dump(const string& header, const UInt8* data, UInt32 size) {
	Buffer out;
	std::lock_guard<std::mutex> lock(_Mutex);
	Util::Dump(data, (_DumpLimit<0 || size<_DumpLimit) ? size : _DumpLimit, out);
	if (_PLogger)
		_PLogger->dump(header, out.data(), out.size());
	else
		_DefaultLogger.dump(header, out.data(), out.size());
}


} // namespace Mona
