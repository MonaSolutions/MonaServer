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

using namespace std;
using namespace Mona;



void LUAPublication::AddListener(lua_State* pState, UInt8 indexPublication, UInt8 indexListener) {
	// -1 must be the client table!
	Script::Collection(pState, indexPublication, "listeners");
	lua_pushvalue(pState, indexListener);
	lua_pushvalue(pState, -4); // client table
	Script::FillCollection(pState, 1);
	lua_pop(pState, 1);
}

void LUAPublication::RemoveListener(lua_State* pState, const Publication& publication) {
	// -1 must be the listener table!
	if (Script::FromObject<Publication>(pState, publication)) {
		Script::Collection(pState, -1, "listeners");
		lua_pushvalue(pState, -3); // listener table
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
	if (!publication.publisher())
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
		} else if (publication.publisher())
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

int	LUAPublication::Flush(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		publication.flush();
	SCRIPT_CALLBACK_RETURN
}


int	LUAPublication::PushData(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		Client* pPublisher = publication.publisher();
		if (pPublisher) {
			std::shared_ptr<DataWriter> pWriter;
			pPublisher->writer().createWriter(pWriter);
			if (pWriter) {
				UInt32 offset = pWriter->packet.size();
				SCRIPT_READ_DATA(*pWriter)
				PacketReader packet(pWriter->packet.data(), pWriter->packet.size());
				packet.next(offset);
				std::shared_ptr<DataReader> pReader;
				pPublisher->writer().createReader(packet, pReader);
				if (pReader)
					publication.pushData(*pReader);
				else
					SCRIPT_ERROR("The publisher of ", publication.name(), " publication has no reader type to push data, use a typed pushData version rather");
			} else
				SCRIPT_ERROR("The publisher of ", publication.name(), " publication has no writer type to push data, use a typed pushData version rather");
		} else
			SCRIPT_ERROR("No data can be pushed on the ", publication.name(), " publication without a publisher")
	SCRIPT_CALLBACK_RETURN
}


int LUAPublication::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		const char* name = SCRIPT_READ_STRING(NULL);
		if(name) {
			if(strcmp(name,"publisher")==0) {
				if (publication.publisher())
					Script::AddObject<LUAClient>(pState, *publication.publisher()); // can change
			} else if(strcmp(name,"name")==0) {
				SCRIPT_WRITE_STRING(publication.name().c_str())
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"listeners")==0) {
				Script::Collection(pState, 1, "listeners");
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"audioQOS")==0) {
				Script::AddObject<LUAQualityOfService>(pState, publication.audioQOS());
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"videoQOS")==0) {
				Script::AddObject<LUAQualityOfService>(pState, publication.videoQOS());
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"dataQOS")==0) {
				Script::AddObject<LUAQualityOfService>(pState, publication.dataQOS());
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"close")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::Close)
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"pushAudio")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::PushAudio)
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"flush")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::Flush)
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"pushVideo")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::PushVideo)
				SCRIPT_CALLBACK_FIX_INDEX
			} else if(strcmp(name,"pushData")==0) {
				SCRIPT_WRITE_FUNCTION(LUAPublication::PushData)
				SCRIPT_CALLBACK_FIX_INDEX
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAPublication::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Publication, publication)
		lua_rawset(pState, 1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
