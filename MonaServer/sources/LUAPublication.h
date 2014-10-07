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

class LUAPublication {
public:
	// -1 must be the client table!
	static void AddListener(lua_State* pState, Mona::UInt8 indexPublication, Mona::UInt8 indexListener);
	// -1 must be the listener table!
	static void RemoveListener(lua_State* pState, const Mona::Publication& publication);



	static void Init(lua_State *pState, Mona::Publication& publication) {}
	static void Clear(lua_State* pState, Mona::Publication& publication);
	static void Delete(lua_State* pState, Mona::Publication& publication);
	static int Get(lua_State* pState);
	static int Set(lua_State* pState);

	
private:
	
	static int PushVideo(lua_State *pState);
	static int PushAudio(lua_State *pState);
	static int PushAMF0Data(lua_State *pState);
	static int Flush(lua_State *pState);
	static int Close(lua_State *pState);
	// metadata
	static int WriteProperties(lua_State *pState);
	static int ClearProperties(lua_State *pState);

	template<typename DataReaderType>
	static int	PushData(lua_State *pState) {
		SCRIPT_CALLBACK(Mona::Publication, publication)
			if (publication.running()) {
				SCRIPT_READ_BINARY(data,size)
				Mona::PacketReader packet(data, size);
				DataReaderType reader(packet);
				publication.pushData(reader);
			} else
				SCRIPT_ERROR("No data can be pushed on ", publication.name(), " publication stopped")
		SCRIPT_CALLBACK_RETURN
	}

	template<typename DataReaderType>
	static int	PushDataWithBuffers(lua_State *pState) {
		SCRIPT_CALLBACK(Mona::Publication, publication)
			if (publication.running()) {
				SCRIPT_READ_BINARY(data,size)
				Mona::PacketReader packet(data, size);
				DataReaderType reader(packet,publication.poolBuffers);
				publication.pushData(reader);
			} else
				SCRIPT_ERROR("No data can be pushed on ", publication.name(), " publication stopped")
		SCRIPT_CALLBACK_RETURN
	}

};
