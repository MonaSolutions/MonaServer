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

Logger*			Logs::_PLogger(NULL);
Logs::DumpMode	Logs::_DumpMode(DUMP_NOTHING);
Int16			Logs::_DumpLimit(-1);
#if defined(_DEBUG)
UInt8			Logs::_Level(Logger::LEVEL_DEBUG); // default log level
#else
UInt8			Logs::_Level(Logger::LEVEL_INFO); // default log level
#endif
Logger			Logs::_DefaultLogger;

} // namespace Mona
