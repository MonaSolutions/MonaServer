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

#include "LUAGroups.h"
#include "LUAGroup.h"
#include "LUAMember.h"
#include "Mona/Invoker.h"
#include "Mona/Util.h"
#include "Poco/HexBinaryDecoder.h"
#include <sstream>

using namespace Mona;
using namespace Poco;
using namespace std;

const char*		LUAGroups::Name="Mona::Entities<Group>";


int LUAGroups::Pairs(lua_State* pState) {
	SCRIPT_CALLBACK(Entities<Group>,LUAGroups,groups)
		lua_getglobal(pState,"next");
		if(!lua_iscfunction(pState,-1))
			SCRIPT_ERROR("'next' should be a LUA function, it should not be overloaded")
		else {
			lua_newtable(pState);
			Entities<Group>::Iterator it;
			for(it=groups.begin();it!=groups.end();++it) {
				SCRIPT_WRITE_PERSISTENT_OBJECT(Group,LUAGroup,*it->second)
				lua_setfield(pState,-2,Mona::Util::FormatHex(it->second->id,ID_SIZE).c_str());
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAGroups::Join(lua_State* pState) {
	SCRIPT_CALLBACK(Entities<Group>,LUAGroups,groups)

		lua_getglobal(pState,"mona");
		Handler* pHandler = NULL;
		if(lua_getmetatable(pState,-1)!=0) {
			lua_getfield(pState,-1,"__this");
			if(lua_islightuserdata(pState,-1))
				pHandler = (Handler*) lua_touserdata(pState, -1);
			lua_pop(pState,2);
		}
		lua_pop(pState,1);

		if(pHandler) {
			SCRIPT_READ_BINARY(peerId,size)
			if(size==(ID_SIZE*2)) {
				stringstream ss;
				ss.write((const char*)peerId,size);
				HexBinaryDecoder(ss).read((char*)peerId,ID_SIZE);
			} else if(size!=ID_SIZE) {
				pHandler=NULL;
				if(peerId)
					SCRIPT_ERROR("Bad member format id ",Mona::Util::FormatHex(peerId,size))
				else
					SCRIPT_ERROR("Member id argument missing")
			}

			if(pHandler) {
				SCRIPT_READ_BINARY(groupId,size)
				if(size==(ID_SIZE*2)) {
					stringstream ss;
					ss.write((const char*)groupId,size);
					HexBinaryDecoder(ss).read((char*)groupId,ID_SIZE);
				} else if(size!=ID_SIZE) {
					pHandler=NULL;
					if(groupId)
						SCRIPT_ERROR("Bad group format id ",Mona::Util::FormatHex(groupId,size))
					else
						SCRIPT_ERROR("Group id argument missing")
				}

				if(pHandler) {
					Peer* pPeer = new Peer(*pHandler);
					memcpy((void*)pPeer->id,peerId,ID_SIZE);
					pPeer->joinGroup(groupId,NULL);
					SCRIPT_WRITE_OBJECT(Peer,LUAMember,*pPeer)
					SCRIPT_ADD_DESTRUCTOR(&LUAMember::Destroy)
				}
			}
			
		} else
			SCRIPT_CRITIC("Impossible to find the handler associated")
	SCRIPT_CALLBACK_RETURN
}


int LUAGroups::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Entities<Group>,LUAGroups,groups)
		string name = SCRIPT_READ_STRING("");
		if(name=="pairs")
			SCRIPT_WRITE_FUNCTION(&LUAGroups::Pairs)
		else if(name=="count")
			SCRIPT_WRITE_NUMBER(groups.count())
		else if(name=="join")
			SCRIPT_WRITE_FUNCTION(&LUAGroups::Join)
		else if(name=="(") {
			SCRIPT_READ_BINARY(id,size)
			Group* pGroup = NULL;
			if(size==ID_SIZE)
				pGroup = groups(id);
			else if(size==(ID_SIZE*2)) {
				stringstream ss;
				ss.write((const char*)id,size);
				HexBinaryDecoder(ss).read((char*)id,ID_SIZE);
				pGroup = groups(id);
			} else if(id)
				SCRIPT_ERROR("Bad group format id ",Mona::Util::FormatHex(id,size))
			else
				SCRIPT_ERROR("Group id argument missing")
			if(pGroup)
				SCRIPT_WRITE_PERSISTENT_OBJECT(Group,LUAGroup,*pGroup)
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAGroups::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Entities<Group>,LUAGroups,groups)
		string name = SCRIPT_READ_STRING("");
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}
