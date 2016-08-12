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

#include "LUAMediaWriter.h"

using namespace std;
using namespace Mona;

int LUAMediaWriter::Write(lua_State* pState) {
	SCRIPT_CALLBACK(LUAMediaWriter, writer)
		UInt8 track(SCRIPT_READ_UINT(0));
		UInt32 time(SCRIPT_READ_UINT(0));
		SCRIPT_READ_BINARY(data, size)

		if(writer._pMedia) {
			writer.clear();
			if (writer._first) { // Write headers
				writer._first = false;
				writer._pMedia->write(writer);
			} 
			writer._pMedia->write(writer, track, time, data, size);
			SCRIPT_WRITE_BINARY(writer.data(), writer.size())
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAMediaWriter::Get(lua_State* pState) {
	SCRIPT_CALLBACK(LUAMediaWriter, writer)
		const char* name = SCRIPT_READ_STRING(NULL);
		if(name) {
			if(strcmp(name,"write")==0) {
				SCRIPT_WRITE_FUNCTION(LUAMediaWriter::Write)
				SCRIPT_FIX_RESULT
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAMediaWriter::Set(lua_State* pState) {
	SCRIPT_CALLBACK(LUAMediaWriter, writer)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
