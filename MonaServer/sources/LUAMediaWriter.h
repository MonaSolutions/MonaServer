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


#include "Mona/PacketWriter.h"
#include "Mona/MediaContainer.h"
#include "Script.h"

using namespace Mona;

class LUAMediaWriter : public PacketWriter {
public:
	LUAMediaWriter(const PoolBuffers& poolBuffers, const char* mediaType) : PacketWriter(poolBuffers), _first(true) {
		if(String::ICompare(mediaType, "mp2t")==0)
			_pMedia.reset(new MPEGTS(poolBuffers));
		else
			_pMedia.reset(new FLV(poolBuffers));
	}

	static int Get(lua_State* pState);
	static int Set(lua_State* pState);

	static void Init(lua_State *pState, LUAMediaWriter& writer) {}
	static void	Clear(lua_State* pState, LUAMediaWriter& writer) {}
	static void	Delete(lua_State* pState, LUAMediaWriter& writer) {}

	virtual ~LUAMediaWriter() {}

private:
	static int	Write(lua_State* pState);
	
	std::unique_ptr<MediaContainer>		_pMedia;
	bool								_first;
};
