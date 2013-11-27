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

#include "Script.h"
#include "Mona/Invoker.h"
#include "Mona/Publication.h"
#include "LUAListener.h"
#include "LUAQualityOfService.h"
#include "LUAClient.h"

class LUAMyPublication;
class LUAPublicationBase {
public:
	static void Clear(lua_State* pState, const Mona::Publication& publication);

	static int Item(lua_State *pState);


	static void AddListener(lua_State* pState, const Mona::Listener& listener, int indexListener, int indexClient);
	static void RemoveListener(lua_State* pState, const Mona::Listener& listener, int indexListener);

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
			std::string name = SCRIPT_READ_STRING("");
			Mona::Publication* pPublication = Publication(publication);
			if (!pPublication) {
				SCRIPT_ERROR("Publication ", name,", publication closed")
			} else {
				string name = SCRIPT_READ_STRING("");
				if(name=="publisher") {
					if (pPublication->publisher())
						SCRIPT_ADD_OBJECT(Client, LUAClient, *pPublication->publisher())
				} else if(name=="name") {
					SCRIPT_WRITE_STRING(pPublication->name().c_str())
				} else if(name=="listeners") {
					Script::Collection(pState, 1, "listeners", pPublication->listeners.count());
				} else if(name=="droppedFrames") {
					SCRIPT_WRITE_NUMBER(pPublication->droppedFrames())
				} else if(name=="audioQOS") {
					SCRIPT_ADD_OBJECT(QualityOfService, LUAQualityOfService, pPublication->audioQOS())
				} else if(name=="videoQOS") {
					SCRIPT_ADD_OBJECT(QualityOfService,LUAQualityOfService,pPublication->videoQOS())
				} else if(name=="dataQOS") {
					SCRIPT_ADD_OBJECT(QualityOfService,LUAQualityOfService,pPublication->dataQOS())
				} else if(name=="close") {
					SCRIPT_WRITE_FUNCTION(&LUAPublication::Close)
				} else if(name=="pushAudio") {
					SCRIPT_WRITE_FUNCTION(&LUAPublication::PushAudio)
				} else if(name=="flush") {
					SCRIPT_WRITE_FUNCTION(&LUAPublication::Flush)
				} else if(name=="pushVideo") {
					SCRIPT_WRITE_FUNCTION(&LUAPublication::PushVideo)
				} else if(name=="pushData") {
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
				SCRIPT_READ_BINARY(pData, size)
				Mona::UInt32 time = SCRIPT_READ_UINT(0);
				if (pData) {
					Mona::MemoryReader reader(pData, size);
					pPublication->pushAudio(reader, time, SCRIPT_READ_UINT(0));
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
				SCRIPT_READ_BINARY(pData, size)
				Mona::UInt32 time = SCRIPT_READ_UINT(0);
				if (pData) {
					Mona::MemoryReader reader(pData, size);
					pPublication->pushVideo(reader, time, SCRIPT_READ_UINT(0));
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
						Mona::UInt32 offset = pWriter->stream.size();
						SCRIPT_READ_DATA(*pWriter)
							Mona::MemoryReader reader(pWriter->stream.data(), pWriter->stream.size());
						reader.next(offset);
						std::shared_ptr<Mona::DataReader> pReader;
						pPublisher->writer().createReader(reader, pReader);
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
