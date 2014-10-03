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

#include "Mona/XMLParser.h"
#include "Script.h"

class LUAXML : private Mona::XMLParser {
/*
XML

<?xml version="1.0"?>
<document>
  <article>
    <p>This is the first paragraph.</p>
    <h2 class='opt'>Title with opt style</h2>
  </article>
  <article>
    <p>Some <b>important</b> text.</p>
  </article>
</document>

LUA

{ xml = {version = 1.0},
	{__name = 'document',
	  {__name = 'article',
		{__name = 'p', 'This is the first paragraph.'},
		{__name = 'h2', class = 'opt', 'Title with opt style'},
	  },
	  {__name = 'article',
		{__name = 'p', 'Some ', {__name = 'b', 'important'}, ' text.'},
	  },
	}
}


Access to "This is the first paragraph" =>
1 - variable[1][1][1][1]
2 - variable.document.article.p.__value

*/

public:
	
	static void XMLToLUA(Mona::Exception& ex, lua_State* pState, const char* data, Mona::UInt32 size, const Mona::PoolBuffers& poolBuffers) { LUAXML(pState, data,size,poolBuffers).parse(ex); }
	static bool LUAToXML(Mona::Exception& ex, lua_State* pState, int index, const Mona::PoolBuffers& poolBuffers);

private:
	static bool LUAToXMLElement(Mona::Exception& ex, lua_State* pState, Mona::PacketWriter& writer);


	LUAXML(lua_State* pState, const char* data, Mona::UInt32 size, const Mona::PoolBuffers& poolBuffers) : _pState(pState), XMLParser(data, size, poolBuffers) {}

	bool onStartXMLDocument() { lua_newtable(_pState); _firstIndex = lua_gettop(_pState); return true; }
	bool onStartXMLElement(const char* name, Mona::Parameters& attributes);
	bool onXMLInfos(const char* name, Mona::Parameters& attributes);
	bool onInnerXMLElement(const char* name, const char* data, Mona::UInt32 size);
	bool onEndXMLElement(const char* name);
	void onEndXMLDocument(const char* error) { addMetatable(); }

	void addMetatable();

	static int Index(lua_State* pState);
	static int NewIndex(lua_State* pState);

	lua_State*			_pState;
	int					_firstIndex;
};
