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

#include "LUAQualityOfService.h"


using namespace std;
using namespace Mona;


int LUAQualityOfService::Get(lua_State *pState) {
	SCRIPT_CALLBACK(QualityOfService,qos)
		const char* name = SCRIPT_READ_STRING(NULL);
		if(name) {
			if (strcmp(name,"lostRate")==0) {
				SCRIPT_WRITE_NUMBER(qos.lostRate) // change
			} else if (strcmp(name, "byteRate") == 0) {
				SCRIPT_WRITE_NUMBER(qos.byteRate) // change
			} else if (strcmp(name, "latency") == 0) {
				SCRIPT_WRITE_NUMBER(qos.latency) // change
			} else if(strcmp(name,"lastSendingTime")==0) {
				SCRIPT_WRITE_NUMBER(qos.lastSendingTime)  // can change
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAQualityOfService::Set(lua_State *pState) {
	SCRIPT_CALLBACK(QualityOfService,qos)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}

