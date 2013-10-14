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

#include "LUAQualityOfService.h"
#include "Mona/QualityOfService.h"

using namespace std;
using namespace Mona;

const char*		LUAQualityOfService::Name="Mona::QualityOfService";


int LUAQualityOfService::Get(lua_State *pState) {
	SCRIPT_CALLBACK(QualityOfService,LUAQualityOfService,qos)
		string name = SCRIPT_READ_STRING("");
		if(name=="lostRate") {
			SCRIPT_WRITE_NUMBER(qos.lostRate)
		} else if(name=="byteRate") {
			SCRIPT_WRITE_NUMBER(qos.byteRate)
		} else if(name=="latency") {
			SCRIPT_WRITE_NUMBER(qos.latency)
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAQualityOfService::Set(lua_State *pState) {
	SCRIPT_CALLBACK(QualityOfService,LUAQualityOfService,qos)
		string name = SCRIPT_READ_STRING("");
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}

