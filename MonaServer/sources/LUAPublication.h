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

#include "Mona/Invoker.h"
#include "Mona/Publication.h"
#include "LUAListener.h"
#include "LUAQualityOfService.h"
#include "LUAClient.h"
#include "Script.h"

class LUAMyPublication;
class LUAPublicationBase {
public:
	static void Clear(lua_State* pState, const Mona::Publication& publication);

	static int Item(lua_State *pState);

	// -1 must be the client table!
	static void AddListener(lua_State* pState, const Mona::Listener& listener, Mona::UInt8 indexListener);
	// -1 must be the listener table!
	static void RemoveListener(lua_State* pState, const Mona::Listener& listener);

	static void Init(lua_State *pState, Mona::Publication& publication) {}
protected:
	static Mona::Publication* Publication(Mona::Publication& publication) { return &publication; }
	static Mona::Publication* Publication(LUAMyPublication& luaPublication);

	static void Close(lua_State *pState,Mona::Publication& publication);
	static void Close(lua_State *pState, LUAMyPublication& luaPublication);
};

template<class PublicationType = Mona::Publication>
class LUAPublication : public LUAPublicationBase {
public:

	static void Init(lua_State *pState, PublicationType& publication) {
		Mona::Publication* pPublication = Publication(publication);
		if (pPublication)
			LUAPublicationBase::Init(pState, *pPublication);
	}

	static int Get(lua_State *pState) {
		SCRIPT_CALLBACK(PublicationType, publication)
			const char* name = SCRIPT_READ_STRING("");
			Mona::Publication* pPublication = Publication(publication);
			if (!pPublication) {
				SCRIPT_ERROR("Publication ", name,", publication closed")
			} else {
				if(strcmp(name,"publisher")==0) {
					if (pPublication->publisher())
						SCRIPT_ADD_OBJECT(Mona::Client, LUAClient, *pPublication->publisher())
				} else if(strcmp(name,"name")==0) {
					SCRIPT_WRITE_STRING(pPublication->name().c_str())
				} else if(strcmp(name,"listeners")==0) {
					Script::Collection(pState, 1, "listeners", pPublication->listeners.count());
				} else if(strcmp(name,"audioQOS")==0) {
					SCRIPT_ADD_OBJECT(Mona::QualityOfService, LUAQualityOfService, pPublication->audioQOS())
				} else if(strcmp(name,"videoQOS")==0) {
					SCRIPT_ADD_OBJECT(Mona::QualityOfService,LUAQualityOfService,pPublication->videoQOS())
				} else if(strcmp(name,"dataQOS")==0) {
					SCRIPT_ADD_OBJECT(Mona::QualityOfService,LUAQualityOfService,pPublication->dataQOS())
				} else if(strcmp(name,"close")==0) {
					SCRIPT_WRITE_FUNCTION(&LUAPublication::Close)
				} else if(strcmp(name,"pushAudio")==0) {
					SCRIPT_WRITE_FUNCTION(&LUAPublication::PushAudio)
				} else if(strcmp(name,"flush")==0) {
					SCRIPT_WRITE_FUNCTION(&LUAPublication::Flush)
				} else if(strcmp(name,"pushVideo")==0) {
					SCRIPT_WRITE_FUNCTION(&LUAPublication::PushVideo)
				} else if(strcmp(name,"pushData")==0) {
					SCRIPT_WRITE_FUNCTION(&LUAPublication::PushData)
				}
			}
		SCRIPT_CALLBACK_RETURN
	}

	static int Set(lua_State *pState) {
		SCRIPT_CALLBACK(PublicationType, publication)
			lua_rawset(pState, 1); // consumes key and value
		SCRIPT_CALLBACK_RETURN
	}

private:
	static int Close(lua_State *pState) {
		SCRIPT_CALLBACK(PublicationType, publication)
			LUAPublicationBase::Close(pState,publication);
		SCRIPT_CALLBACK_RETURN
	}

	static int	Flush(lua_State *pState) {
		SCRIPT_CALLBACK(PublicationType, publication)
			Mona::Publication* pPublication = Publication(publication);
			if (!pPublication)
				SCRIPT_ERROR("Publication:flush, publication closed")
			else
				pPublication->flush();
		SCRIPT_CALLBACK_RETURN
	}

	static int PushAudio(lua_State *pState) {
		SCRIPT_CALLBACK(PublicationType, publication)
			Mona::Publication* pPublication = Publication(publication);
			if (!pPublication)
				SCRIPT_ERROR("Publication:pushAudio, publication closed")
			else {
				Mona::UInt32 time = SCRIPT_READ_UINT(0);
				SCRIPT_READ_BINARY(pData, size);
				if (pData) {
					Mona::PacketReader packet(pData, size);
					pPublication->pushAudio(time,packet, SCRIPT_READ_UINT(0));
				}
			}
		SCRIPT_CALLBACK_RETURN
	}

	static int PushVideo(lua_State *pState) {
		SCRIPT_CALLBACK(PublicationType, publication)
			Mona::Publication* pPublication = Publication(publication);
			if (!pPublication)
				SCRIPT_ERROR("Publication:pushVideo, publication closed")
			else {
				Mona::UInt32 time = SCRIPT_READ_UINT(0);
				SCRIPT_READ_BINARY(pData, size);
				if (pData) {
					Mona::PacketReader packet(pData, size);
					pPublication->pushVideo(time,packet, SCRIPT_READ_UINT(0));
				}
			}
		SCRIPT_CALLBACK_RETURN
	}

	static int PushData(lua_State *pState) {
		SCRIPT_CALLBACK(Mona::Publication, publication)
			Mona::Publication* pPublication = Publication(publication);
			if (!pPublication)
				SCRIPT_ERROR("Publication:pushData, publication closed")
			else {
				Mona::Client* pPublisher = publication.publisher();
				if (pPublisher) {
					std::shared_ptr<Mona::DataWriter> pWriter;
					pPublisher->writer().createWriter(pWriter);
					if (pWriter) {
						Mona::UInt32 offset = pWriter->packet.size();
						SCRIPT_READ_DATA(*pWriter)
						Mona::PacketReader packet(pWriter->packet.data(), pWriter->packet.size());
						packet.next(offset);
						std::shared_ptr<Mona::DataReader> pReader;
						pPublisher->writer().createReader(packet, pReader);
						if (pReader)
							pPublication->pushData(*pReader);
						else
							SCRIPT_ERROR("The publisher of ", publication.name(), " publication has no reader type to push data, use a typed pushData version rather");
					} else
						SCRIPT_ERROR("The publisher of ", publication.name(), " publication has no writer type to push data, use a typed pushData version rather");
				} else
					SCRIPT_ERROR("No data can be pushed on the ", publication.name(), " publication without a publisher")
			}
		SCRIPT_CALLBACK_RETURN
	}

};

class LUAMyPublication : public LUAPublication<LUAMyPublication> {
public:
	LUAMyPublication(Mona::Publication& publication, Mona::Invoker& invoker) : closed(false), invoker(invoker), publication(publication) {}

	static void Create(lua_State *pState, LUAMyPublication& publication) {}
	static int	Destroy(lua_State* pState);

	Mona::Publication&	publication;
	Mona::Invoker&		invoker;
	bool				closed;
};
