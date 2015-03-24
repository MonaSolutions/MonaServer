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

#include "Mona/RTSP/RTSP.h"


namespace Mona {

const char* RTSP::MAP_COMMANDS[] ={
	"",
	"OPTIONS", 
	"DESCRIBE",
	"SETUP",
	"PLAY",
	"GET_PARAMETER",
	"SET_PARAMETER",
	"TEARDOWN",
};

RTSP::CommandType RTSP::ParseCommand(Exception& ex,const char* value) {
	if (!value)
		return COMMAND_UNKNOWN;

	for(UInt8 index = 1; index < 8; index++) {
		if (String::ICompare(value,MAP_COMMANDS[index],strlen(MAP_COMMANDS[index]))==0)
			return (CommandType)index;
	}
	ex.set(Exception::PROTOCOL, "Unknown RTSP command ", value);
	return COMMAND_UNKNOWN;
}

} // namespace Mona
