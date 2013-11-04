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

#include "LUAWriter.h"
#include "Service.h"
#include "LUAQualityOfService.h"

using namespace std;
using namespace Mona;



LUAWriter::LUAWriter(lua_State* pState,Writer& writer):writer(writer.newWriter(this)),_pState(pState) {
}

void LUAWriter::close(Writer& writer,int code){
	SCRIPT_BEGIN(_pState)
		Script::ClearPersistentObject<QualityOfService,LUAQualityOfService>(_pState,writer.qos());
		Script::ClearPersistentObject<Writer,LUAWriter>(_pState,writer);
	SCRIPT_END
	delete this;
}

const char*			LUAWriter::Name="Mona::Writer";

int LUAWriter::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(Writer,LUAWriter,writer)
		writer.close();
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::Close(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,LUAWriter,writer)
		writer.close(SCRIPT_READ_INT(0));
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Writer,LUAWriter,writer)
		string name = SCRIPT_READ_STRING("");
		if(name=="reliable") {
			SCRIPT_WRITE_BOOL(writer.reliable)
		} else if(name=="flush") {
			SCRIPT_WRITE_FUNCTION(&LUAWriter::Flush)
		} else if(name=="writeMessage") {
			SCRIPT_WRITE_FUNCTION(&LUAWriter::WriteMessage)
		} else if(name=="writeInvocation") {
			SCRIPT_WRITE_FUNCTION(&LUAWriter::WriteInvocation)
		} else if(name=="writeRaw") {
			SCRIPT_WRITE_FUNCTION(&LUAWriter::WriteRaw)
		} else if(name=="newWriter") {
			SCRIPT_WRITE_FUNCTION(&LUAWriter::NewWriter)
		} else if(name=="qos") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(QualityOfService,LUAQualityOfService,writer.qos())
		} else if(name=="close") {
			SCRIPT_WRITE_FUNCTION(&LUAWriter::Close)
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Writer,LUAWriter,writer)
		string name = SCRIPT_READ_STRING("");
		if(name=="reliable")
			writer.reliable = lua_toboolean(pState,-1)==0 ? false : true;
		else
			lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::Flush(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,LUAWriter,writer)
		writer.flush(SCRIPT_READ_BOOL(true));
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::WriteMessage(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,LUAWriter,writer)
		SCRIPT_READ_DATA(writer.writeMessage())
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::WriteInvocation(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,LUAWriter,writer)
		SCRIPT_READ_DATA(writer.writeInvocation(SCRIPT_READ_STRING("")))
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::WriteRaw(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,LUAWriter,writer)
		SCRIPT_READ_BINARY(data,size);
		writer.writeRaw(data,size);
	SCRIPT_CALLBACK_RETURN
}


int LUAWriter::NewWriter(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,LUAWriter,writer)
		SCRIPT_WRITE_PERSISTENT_OBJECT(Writer,LUAWriter,(new LUAWriter(pState,writer))->writer)
		SCRIPT_ADD_DESTRUCTOR(&LUAWriter::Destroy)
	SCRIPT_CALLBACK_RETURN
}
