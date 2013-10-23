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

#include "LUAPublication.h"
#include "LUAListeners.h"
#include "LUAQualityOfService.h"
#include "LUAClient.h"
#include "Mona/JSONWriter.h"
#include "Mona/JSONReader.h"

using namespace std;
using namespace Mona;
using namespace Poco;

const char*		LUAPublication::Name="Publication";

void LUAPublication::Clear(lua_State* pState,const Publication& publication){
	Script::ClearPersistentObject<QualityOfService,LUAQualityOfService>(pState,publication.dataQOS());
	Script::ClearPersistentObject<QualityOfService,LUAQualityOfService>(pState,publication.audioQOS());
	Script::ClearPersistentObject<QualityOfService,LUAQualityOfService>(pState,publication.videoQOS());
	Script::ClearPersistentObject<Listeners,LUAListeners>(pState,publication.listeners);
	Script::ClearPersistentObject<Publication,LUAPublication>(pState,publication);
}

int	LUAPublication::Close(lua_State *pState) {
	SCRIPT_CALLBACK(Publication,LUAPublication,publication)
		lua_getmetatable(pState,1);
		lua_getfield(pState,-1,"__invoker");
		lua_replace(pState,-2);
		if(lua_islightuserdata(pState,-1))
			((Invoker*)lua_touserdata(pState,-1))->unpublish(publication.name());
		else
			SCRIPT_ERROR("You have not the handle on publication ",publication.name(),", you can't close it")
		lua_pop(pState,1);
	SCRIPT_CALLBACK_RETURN
}

int	LUAPublication::Flush(lua_State *pState) {
	SCRIPT_CALLBACK(Publication,LUAPublication,publication)
		publication.flush();
	SCRIPT_CALLBACK_RETURN
}

int	LUAPublication::PushAudio(lua_State *pState) {
	SCRIPT_CALLBACK(Publication,LUAPublication,publication)
		SCRIPT_READ_BINARY(pData,size)
		UInt32 time = SCRIPT_READ_UINT(0);
		if(pData) {
			MemoryReader reader(pData,size);
			publication.pushAudio(reader,time,SCRIPT_READ_UINT(0));
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAPublication::PushVideo(lua_State *pState) {
	SCRIPT_CALLBACK(Publication,LUAPublication,publication)
		SCRIPT_READ_BINARY(pData,size)
		UInt32 time = SCRIPT_READ_UINT(0);
		if(pData) {
			MemoryReader reader(pData,size);
			publication.pushVideo(reader,time,SCRIPT_READ_UINT(0));
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAPublication::PushData(lua_State *pState) {
	SCRIPT_CALLBACK(Publication,LUAPublication,publication)
		Client* pPublisher = publication.publisher();
		if(pPublisher) {
			SharedPtr<DataWriter> pWriter;
			pPublisher->writer().createWriter(pWriter);
			if(!pWriter.isNull()) {
				UInt32 offset = pWriter->stream.size();
				SCRIPT_READ_DATA(*pWriter)
				MemoryReader reader(pWriter->stream.data(),pWriter->stream.size());
				reader.next(offset);
				SharedPtr<DataReader> pReader;
				pPublisher->writer().createReader(reader,pReader);
				if(!pReader.isNull())
					publication.pushData(*pReader);
				else
					SCRIPT_ERROR("The publisher of ",publication.name()," publication has no reader type to push data, use a typed pushData version rather");
			} else
				SCRIPT_ERROR("The publisher of ",publication.name()," publication has no writer type to push data, use a typed pushData version rather");
		} else
			SCRIPT_ERROR("No data can be pushed on the ",publication.name()," publication without a publisher")
	SCRIPT_CALLBACK_RETURN
}

int	LUAPublication::PushJSONData(lua_State *pState) {
	SCRIPT_CALLBACK(Publication,LUAPublication,publication)
		JSONWriter writer;
		UInt32 offset = writer.stream.size();
		SCRIPT_READ_DATA(writer)
		MemoryReader reader(writer.stream.data(),writer.stream.size());
		reader.next(offset);
		JSONReader data(reader);
		publication.pushData(data);
	SCRIPT_CALLBACK_RETURN
}

int	LUAPublication::PushAMFData(lua_State *pState) {
	SCRIPT_CALLBACK(Publication,LUAPublication,publication)
		AMFWriter writer;
		UInt32 offset = writer.stream.size();
		SCRIPT_READ_DATA(writer)
		MemoryReader reader(writer.stream.data(),writer.stream.size());
		reader.next(offset);
		AMFReader data(reader);
		publication.pushData(data);
	SCRIPT_CALLBACK_RETURN
}


int LUAPublication::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Publication,LUAPublication,publication)
		string name = SCRIPT_READ_STRING("");
		if(name=="publisher") {
			if(publication.publisher())
				SCRIPT_WRITE_PERSISTENT_OBJECT(Client,LUAClient,*publication.publisher())
		} else if(name=="name") {
			SCRIPT_WRITE_STRING(publication.name().c_str())
		} else if(name=="listeners") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(Listeners,LUAListeners,publication.listeners)
		} else if(name=="droppedFrames") {
			SCRIPT_WRITE_NUMBER(publication.droppedFrames())
		} else if(name=="audioQOS") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(QualityOfService,LUAQualityOfService,publication.audioQOS())
		} else if(name=="videoQOS") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(QualityOfService,LUAQualityOfService,publication.videoQOS())
		} else if(name=="dataQOS") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(QualityOfService,LUAQualityOfService,publication.dataQOS())
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
		} else if(name=="pushAMFData") {
			SCRIPT_WRITE_FUNCTION(&LUAPublication::PushAMFData)
		} else if(name=="pushJSONData") {
			SCRIPT_WRITE_FUNCTION(&LUAPublication::PushJSONData)
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAPublication::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Publication,LUAPublication,publication)
		string name = SCRIPT_READ_STRING("");
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}

