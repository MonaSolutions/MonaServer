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

#include "LUAWriter.h"
#include "Service.h"
#include "LUAQualityOfService.h"

using namespace std;
using namespace Mona;



LUAWriter::LUAWriter(lua_State* pState,Writer& writer):writer(writer),_pState(pState) {
	onClose = [this](Int32 code) {
		Clear(_pState, this->writer);
		this->writer.unsubscribe(onClose);
		delete this; // delete LUAWriter instance!
	};
	writer.subscribe(onClose);
}

int LUAWriter::Destroy(lua_State* pState) {
	SCRIPT_DESTRUCTOR_CALLBACK(Writer,writer)
		writer.close();
	SCRIPT_CALLBACK_RETURN
}

void LUAWriter::Clear(lua_State* pState,const Writer& writer){
	Script::ClearObject<QualityOfService, LUAQualityOfService>(pState, writer.qos());
}

int LUAWriter::Close(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,writer)
		writer.close(SCRIPT_READ_UINT(0));
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Writer,writer)
		const char* name = SCRIPT_READ_STRING(NULL);
		if(name) {
			if(strcmp(name,"reliable")==0) {
				SCRIPT_WRITE_BOOL(writer.reliable)
			} else if (strcmp(name, "flush") == 0) {
				SCRIPT_WRITE_FUNCTION(&LUAWriter::Flush)
			} else if (strcmp(name, "writeMessage") == 0) {
				SCRIPT_WRITE_FUNCTION(&LUAWriter::WriteMessage)
			} else if (strcmp(name, "writeInvocation") == 0) {
				SCRIPT_WRITE_FUNCTION(&LUAWriter::WriteInvocation)
			} else if (strcmp(name, "writeRaw") == 0) {
				SCRIPT_WRITE_FUNCTION(&LUAWriter::WriteRaw)
			} else if (strcmp(name, "newWriter") == 0) {
				SCRIPT_WRITE_FUNCTION(&LUAWriter::NewWriter)
			} else if (strcmp(name, "qos") == 0) {
				SCRIPT_ADD_OBJECT(QualityOfService,LUAQualityOfService,writer.qos())
			} else if (strcmp(name, "close") == 0) {
				SCRIPT_WRITE_FUNCTION(&LUAWriter::Close)
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Writer,writer)
		const char* name = SCRIPT_READ_STRING("");
		if (strcmp(name, "reliable") == 0)
			writer.reliable = lua_toboolean(pState,-1)==0 ? false : true;
		else
			lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::Flush(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,writer)
		if(writer.state()==Writer::OPENING)
			SCRIPT_ERROR("Violation policy, impossible to flush data on a opening writer")
		else
			writer.flush(SCRIPT_READ_BOOL(true));
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::WriteMessage(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,writer)
		SCRIPT_READ_DATA(writer.writeMessage())
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::WriteInvocation(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,writer)
		SCRIPT_READ_DATA(writer.writeInvocation(SCRIPT_READ_STRING("")))
	SCRIPT_CALLBACK_RETURN
}

int LUAWriter::WriteRaw(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,writer)
		while(SCRIPT_CAN_READ) {
			SCRIPT_READ_BINARY(data, size);
			if (data)
				writer.writeRaw(data, size);
		}
	SCRIPT_CALLBACK_RETURN
}


int LUAWriter::NewWriter(lua_State* pState) {
	SCRIPT_CALLBACK(Writer,writer)
		SCRIPT_NEW_OBJECT(Writer,LUAWriter,&(new LUAWriter(pState,writer.newWriter()))->writer)
	SCRIPT_CALLBACK_RETURN
}
