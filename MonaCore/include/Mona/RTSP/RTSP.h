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
#include "Mona/Exceptions.h"

namespace Mona {

class RTSP : virtual Static {
public:

	enum CommandType {
		COMMAND_UNKNOWN = 0,
		COMMAND_OPTIONS,
		COMMAND_DESCRIBE,
		COMMAND_SETUP,
		COMMAND_PLAY,
		COMMAND_GET_PARAMETER,
		COMMAND_SET_PARAMETER,
		COMMAND_TEARDOWN,
	};

	static CommandType ParseCommand(Exception& ex,const char* value);

private:

	static const char* MAP_COMMANDS[];
};




} // namespace Mona
