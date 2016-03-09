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


#include "LUAPublication.h"
#include "Mona/JSONReader.h"
#include "Mona/AMFReader.h"
#include "Mona/StringReader.h"
#include "Mona/XMLRPCReader.h"
#include "ScriptReader.h"

using namespace std;
using namespace Mona;



void LUAPublication::AddListener(lua_State* pState, UInt8 indexPublication, UInt8 indexListener) {
	// -1 must be the client table!
	Script::Collection(pState, indexPublication, "listeners");
	lua_pushvalue(pState, -2); // client table
	lua_pushvalue(pState, indexListener);
	Script::FillCollection(pState, 1);
	lua_pop(pState, 1);
}

void LUAPublication::RemoveListener(lua_State* pState, const Publication& publication) {
	// -1 must be the client table!
	if (Script::FromObject<Publication>(pState, publication)) {
		Script::Collection(pState, -1, "listeners");
		lua_pushvalue(pState, -3); // client table
		lua_pushnil(pState);
		Script::FillCollection(pState, 1);
		lua_pop(pState, 2);
	}
}

void LUAPublication::Clear(lua_State* pState, Publication& publication) {
	Script::ClearObject<LUAQualityOfService>(pState, publication.dataQOS());
	Script::ClearObject<LUAQualityOfService>(pState, publication.audioQOS());
	Script::ClearObject<LUAQualityOfService>(pState, publication.videoQOS());
}

void LUAPublication::Delete(lua_State* pState, Publication& publication) {
	if (!publication.running())
		return;
	lua_getmetatable(pState, -1);
	lua_getfield(pState, -1, "|invoker");
	lua_replace(pState, -2);
	Invoker* pInvoker = (Invoker*)lua_touserdata(pState, -1);
	if (pInvoker)
		pInvoker->unpublish(publication.name());
	lua_pop(pState, 1);
}

int LUAPublication::Close(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		lua_getmetatable(pState, 1);
		lua_getfield(pState, -1, "|invoker");
		lua_replace(pState, -2);

		Script::DetachDestructor(pState,1);

		Invoker* pInvoker = (Invoker*)lua_touserdata(pState, -1);
		if (!pInvoker) {
			SCRIPT_BEGIN(pState)
				SCRIPT_ERROR("You have not the handle on publication ", publication.name(), ", you can't close it")
			SCRIPT_END
		} else if (publication.running())
			pInvoker->unpublish(publication.name()); // call LUAPublication::Clear (because no destructor)

		lua_pop(pState, 1);
	SCRIPT_CALLBACK_RETURN
}

int	LUAPublication::PushAudio(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		UInt32 time = SCRIPT_READ_UINT(0);
		SCRIPT_READ_BINARY(pData, size);
		if (pData) {
			PacketReader packet(pData, size);
			publication.pushAudio(time,packet, SCRIPT_READ_UINT(0));
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAPublication::PushVideo(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		UInt32 time = SCRIPT_READ_UINT(0);
		SCRIPT_READ_BINARY(pData, size);
		if (pData) {
			PacketReader packet(pData, size);
			publication.pushVideo(time,packet, SCRIPT_READ_UINT(0));
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAPublication::PushAMF0Data(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		if (publication.running()) {
			AMFWriter writer(publication.poolBuffers);
			writer.amf0=true;
			SCRIPT_READ_NEXT(ScriptReader(pState, SCRIPT_READ_AVAILABLE).read(writer));
			PacketReader packet(writer.packet.data(), writer.packet.size());
			AMFReader reader(packet);
			publication.pushData(reader);
		} else
			SCRIPT_ERROR("No data can be pushed on ", publication.name(), " publication stopped")
	SCRIPT_CALLBACK_RETURN
}

int	LUAPublication::Flush(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		publication.flush();
	SCRIPT_CALLBACK_RETURN
}

int LUAPublication::WriteProperties(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		ScriptReader reader(pState, SCRIPT_READ_AVAILABLE);
		publication.writeProperties(reader);
		Script::Collection(pState, 1, "properties");
	SCRIPT_CALLBACK_RETURN
}

int LUAPublication::ClearProperties(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		publication.clearProperties();
	SCRIPT_CALLBACK_RETURN
}


int LUAPublication::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		const char* name = SCRIPT_READ_STRING(NULL);
		if(name) {
			if(strcmp(name,"running")==0) {
				SCRIPT_WRITE_BOOL(publication.running())
			} else if(strcmp(name,"name")==0) {
				SCRIPT_WRITE_STRING(publication.name().c_str())
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"lastTime")==0) {
				SCRIPT_WRITE_NUMBER(publication.lastTime()) // can change
			} else if(strcmp(name,"droppedFrames")==0) {
				SCRIPT_WRITE_NUMBER(publication.droppedFrames()) // can change
			} else if(strcmp(name,"listeners")==0) {
				Script::Collection(pState, 1, "listeners");
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"audioQOS")==0) {
				Script::AddObject<LUAQualityOfService>(pState, publication.audioQOS());
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"videoQOS")==0) {
				Script::AddObject<LUAQualityOfService>(pState, publication.videoQOS());
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"dataQOS")==0) {
				Script::AddObject<LUAQualityOfService>(pState, publication.dataQOS());
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"close")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::Close)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"pushAudio")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::PushAudio)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"flush")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::Flush)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"pushVideo")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::PushVideo)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"pushAMFData")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::PushData<Mona::AMFReader>)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"pushAMF0Data")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::PushAMF0Data)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"pushXMLRPCData")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::PushDataWithBuffers<Mona::XMLRPCReader>)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"pushJSONData")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::PushDataWithBuffers<Mona::JSONReader>)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"pushData")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::PushData<Mona::StringReader>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"writeProperties")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::WriteProperties)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"clearProperties")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::ClearProperties)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"properties")==0) {
				if(Script::GetCollection(pState, 1, "properties")) // if no properties, must returns null
					SCRIPT_FIX_RESULT
			} else if(Script::GetCollection(pState, 1, "properties")) {
				lua_getfield(pState, -1, name);
				lua_replace(pState, -2);
				SCRIPT_FIX_RESULT
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAPublication::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		lua_rawset(pState, 1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
