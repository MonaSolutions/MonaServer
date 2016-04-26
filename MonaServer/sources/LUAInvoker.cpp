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

#include "LUAInvoker.h"
#include "LUAPublication.h"
#include "LUAUDPSocket.h"
#include "LUATCPClient.h"
#include "LUATCPServer.h"
#include "LUAGroup.h"
#include "LUAMember.h"
#include "LUAFile.h"
#include "LUABroadcaster.h"
#include "LUAXML.h"
#include "LUAMediaWriter.h"
#include "MonaServer.h"
#include <openssl/evp.h>
#include "Mona/AMFReader.h"
#include "Mona/JSONReader.h"
#include "Mona/JSONWriter.h"
#include "Mona/XMLRPCReader.h"
#include "Mona/XMLRPCWriter.h"
#include "Mona/QueryWriter.h"
#include "Mona/QueryReader.h"
#include "math.h"


using namespace std;
using namespace Mona;


class LUAClients {
public:
	static int Item(lua_State *pState) {
		SCRIPT_CALLBACK(Invoker, invoker)

			if (lua_isstring(pState, 2)) {
				SCRIPT_READ_BINARY(id, size)
				Client* pClient(NULL);
				UInt8 rawId[32];
				id = Script::ToRawId(id, size, rawId);
				if (id && (pClient = invoker.clients(id)))
					Script::AddObject<LUAClient>(pState, *pClient);
			}

		SCRIPT_CALLBACK_RETURN
	}
};


class LUAGroups {
public:
	static int Item(lua_State *pState) {
		SCRIPT_CALLBACK(Invoker, invoker)

			if (lua_isstring(pState, 2)) {
				SCRIPT_READ_BINARY(id, size)
				Group* pGroup(NULL);
				UInt8 rawId[32];
				id = Script::ToRawId(id, size, rawId);
				if (id && (pGroup = invoker.groups(id)))
					Script::AddObject<LUAGroup>(pState, *pGroup);
			}

		SCRIPT_CALLBACK_RETURN
	}
};


// HERE JUST TO SET THE COLLECTOR FOR EVERY COLLECTIONS
void LUAInvoker::Init(lua_State *pState, Invoker& invoker) {
	Script::Collection<LUAClients>(pState, -1, "clients");
	lua_setfield(pState, -2,"clients");

	Script::Collection<LUAGroups>(pState, -1, "groups");
	lua_setfield(pState, -2,"groups");
	
	Parameters::ForEach pushProperty([pState](const string& key, const string& value) {
		Script::PushKeyValue(pState,key, value);
	});

	// Configs
	Script::Collection(pState,-1, "configs");
	Script::FillCollection(pState, invoker.iterate(pushProperty));
	lua_setfield(pState, -2,"configs");

	// Environement
	Script::Collection(pState,-1, "environment");
	Script::FillCollection(pState, Util::Environment().iterate(pushProperty));
	lua_setfield(pState, -2,"environment");
}


void LUAInvoker::AddClient(lua_State *pState) {
	// -1 must be the client table!
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "clients");
	lua_getfield(pState, -3,"id");
	lua_pushvalue(pState, -4); // client table
	Script::FillCollection(pState,1);
	lua_pop(pState, 2);
}

void LUAInvoker::RemoveClient(lua_State *pState) {
	// -1 must be the client table!
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "clients");
	lua_getmetatable(pState, -3);
	lua_getfield(pState, -1, "|id");
	lua_replace(pState, -2);
	lua_pushnil(pState);
	Script::FillCollection(pState,1);
	lua_pop(pState, 2);
}

void LUAInvoker::AddPublication(lua_State *pState, const Publication& publication) {
	// -1 must be the publication table!
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "publications");
	lua_pushstring(pState,publication.name().c_str());
	lua_pushvalue(pState, -4);
	Script::FillCollection(pState,1);
	lua_pop(pState, 2);
}

void LUAInvoker::RemovePublication(lua_State *pState, const Publication& publication) {
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "publications");
	lua_pushstring(pState,publication.name().c_str());
	lua_pushnil(pState);
	Script::FillCollection(pState,1);
	lua_pop(pState, 2);
}

void LUAInvoker::AddGroup(lua_State *pState) {
	// -1 must be the group table!
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "groups");
	lua_getfield(pState, -3, "id");
	lua_pushvalue(pState, -4);
	Script::FillCollection(pState,1);
	lua_pop(pState, 2);
}

void LUAInvoker::RemoveGroup(lua_State *pState) {
	// -1 must be the group table!
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "groups");
	lua_getmetatable(pState, -3);
	lua_getfield(pState, -1, "|id");
	lua_replace(pState, -2);
	lua_pushnil(pState);
	Script::FillCollection(pState,1);
	lua_pop(pState, 2);
}


int	LUAInvoker::Split(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		const char* expression = SCRIPT_READ_STRING("");
		const char* separator = SCRIPT_READ_STRING("");
		String::ForEach forEach([__pState](UInt32 index,const char* value) {
			SCRIPT_WRITE_STRING(value);
			return true;
		});
		String::Split(expression, separator, forEach,SCRIPT_READ_UINT(0));
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::Dump(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker, invoker)
		SCRIPT_READ_BINARY(name, sizeName)
		const UInt8* data(NULL);
		UInt32 sizeData(0);
		if (SCRIPT_NEXT_TYPE == LUA_TSTRING) {
			data = BIN lua_tostring(pState, ++__args);
			sizeData = lua_objlen(pState, __args);
		}
		if (name) {
			if (data) {
				UInt32 count(SCRIPT_READ_UINT(sizeData));
				const char* header(STR name);
				while (*header && !isspace(*header++));
				const char* endName(header);
				while (*header && isspace(*header++));
				if (*header) {
					SCOPED_STRINGIFY(name,header-endName,Logs::Dump(STR name, data, count > sizeData ? sizeData : count,header))
				} else
					Logs::Dump(STR name, data, count > sizeData ? sizeData : count);
			} else {
#if defined(_DEBUG)
				UInt32 count(SCRIPT_READ_UINT(sizeName));
				Logs::Dump(name, count > sizeName ? sizeName : count);
#else
				SCRIPT_WARN("debugging mona:dump(",name,",...) not removed")
#endif
			}
		}
	SCRIPT_CALLBACK_RETURN
}


int	LUAInvoker::Publish(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
	string name = SCRIPT_READ_STRING("");
	Exception ex;
	
	Publication* pPublication = invoker.publish(ex, name,strcmp(SCRIPT_READ_STRING("live"),"record")==0 ? Publication::RECORD : Publication::LIVE);
	// ex.error already displayed!
	if(pPublication) {
		Script::AddObject<LUAPublication>(pState, *pPublication); // no new because already added by invoker.publish
		Script::AttachDestructor<LUAPublication>(pState, *pPublication); // add a destructor
		Script::ClearObject<LUAPublication>(pState, *pPublication); // remove of registry table (remove just persitent version because there is a destructor now)
		lua_getmetatable(pState, -1);
		lua_pushlightuserdata(pState, &invoker);
		lua_setfield(pState, -2,"|invoker");
		lua_pop(pState, 1);
	}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::AbsolutePath(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker, invoker)
		string path(invoker.rootPath());
		path += '/';
		SCRIPT_WRITE_STRING(FileSystem::MakeFolder(path += SCRIPT_READ_STRING("")).c_str())
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::CreateUDPSocket(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker, invoker)
		Script::NewObject<LUAUDPSocket>(pState, *new LUAUDPSocket(invoker.sockets, SCRIPT_READ_BOOL(false), pState));
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::CreateTCPClient(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		Script::NewObject<LUATCPClient>(pState, *new LUATCPClient(invoker.sockets, pState));
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::CreateTCPServer(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		Script::NewObject<LUATCPServer>(pState, *new LUATCPServer(invoker.sockets, pState));
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::Md5(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		while(SCRIPT_READ_AVAILABLE) {
			SCRIPT_READ_BINARY(data,size)
			if(data) {
				UInt8 result[16];
				EVP_Digest(data,size,result,NULL,EVP_md5(),NULL);
				SCRIPT_WRITE_BINARY(result, 16);
			} else {
				SCRIPT_ERROR("Input MD5 value have to be a string expression")
				SCRIPT_WRITE_NIL
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAInvoker::ListFiles(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker, invoker)
		string directory(invoker.rootPath());
		directory += '/';

		UInt32 index = 0;
		lua_newtable(pState);
		FileSystem::ForEach forEach([&pState, &index](const string& path, UInt16 level){
			Script::NewObject<LUAFile>(pState, *new File(path));
			lua_rawseti(pState,-2,++index);
		});
		
		Exception ex;
		FileSystem::ListFiles(ex, FileSystem::MakeFolder(directory += SCRIPT_READ_STRING("")), forEach);
		if (ex)
			SCRIPT_ERROR(ex.error());
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::Sha256(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		while(SCRIPT_READ_AVAILABLE) {
			SCRIPT_READ_BINARY(data,size)
			if(data) {
				UInt8 result[32];
				EVP_Digest(data,size,result,NULL,EVP_sha256(),NULL);
				SCRIPT_WRITE_BINARY(result,32);
			} else {
				SCRIPT_ERROR("Input SHA256 value have to be a string expression")
				SCRIPT_WRITE_NIL
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::ToAMF0(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		AMFWriter writer(invoker.poolBuffers);
		writer.amf0=true;
		SCRIPT_READ_NEXT(ScriptReader(pState, SCRIPT_READ_AVAILABLE).read(writer));
		SCRIPT_WRITE_BINARY(writer.packet.data(),writer.packet.size())
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::FromXML(lua_State *pState) {
	SCRIPT_CALLBACK_TRY(Invoker, invoker)
		SCRIPT_READ_BINARY(data,size)
		Exception ex;
		int count(lua_gettop(pState));
		LUAXML::XMLToLUA(ex, pState, STR data, size, invoker.poolBuffers);
		if (ex) {
			// erase object written
			lua_pop(pState, lua_gettop(pState) - count);
			SCRIPT_CALLBACK_THROW(ex.error())
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::ToXML(lua_State *pState) {
	SCRIPT_CALLBACK_TRY(Invoker,invoker)
		// TODO compress option
		Exception ex;
		if (LUAXML::LUAToXML(ex, pState, SCRIPT_READ_NEXT(1), invoker.poolBuffers)) {
			if (ex)
				SCRIPT_WARN(ex.error());
		} else {
			SCRIPT_CALLBACK_THROW(ex.error())
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::AddToBlacklist(lua_State* pState) {
	SCRIPT_CALLBACK(Invoker,invoker)	
		while(SCRIPT_READ_AVAILABLE) {
			Exception ex;
			IPAddress address;
			if (LUAIPAddress::Read(ex, pState, SCRIPT_READ_NEXT(1), address)) {
				if (ex)
					SCRIPT_WARN(ex.error())
				invoker.addBanned(address);
			} else
				SCRIPT_ERROR(ex.error())
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::RemoveFromBlacklist(lua_State* pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		while(SCRIPT_READ_AVAILABLE) {
			Exception ex;
			IPAddress address;
			if (LUAIPAddress::Read(ex, pState, SCRIPT_READ_NEXT(1), address)) {
				if (ex)
					SCRIPT_WARN(ex.error())
				invoker.removeBanned(address);
			} else
				SCRIPT_ERROR(ex.error())
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAInvoker::JoinGroup(lua_State* pState) {
	SCRIPT_CALLBACK(Invoker, invoker)

		SCRIPT_READ_BINARY(peerId, size)

		UInt8 rawId[ID_SIZE];
		if (!peerId)
			SCRIPT_ERROR("Member id argument missing")
		else if(!(peerId = Script::ToRawId(peerId, size, rawId)))
			SCRIPT_ERROR("Bad member format id ", string((const char*)peerId, size))
		else {
			SCRIPT_READ_BINARY(groupId, size)
			if (!groupId)
				SCRIPT_ERROR("Group id argument missing")
			else if(!(groupId = Script::ToRawId(groupId, size, rawId)))
				SCRIPT_ERROR("Bad group format id ", string((const char*)groupId, size))
			else {

				Peer* pPeer = new Peer((Handler&)invoker);
				memcpy((void*)pPeer->id, peerId, ID_SIZE);
				Group& group(pPeer->joinGroup(groupId, NULL));
				Script::AddObject<LUAGroup>(pState, group);
				LUAInvoker::AddGroup(pState);
				Script::NewObject<LUAMember>(pState, *pPeer);
				LUAGroup::AddClient(pState, -2);
				lua_replace(pState, -2);

			}
		}

	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::Time(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		SCRIPT_WRITE_NUMBER(Time::Now())
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::CreateMediaWriter(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker, invoker)
		Script::NewObject<LUAMediaWriter>(pState, *new LUAMediaWriter(invoker.poolBuffers, SCRIPT_READ_STRING("")));
	SCRIPT_CALLBACK_RETURN
}

int LUAInvoker::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		const char* name = SCRIPT_READ_STRING(NULL);
		if (name) {
			if(strcmp(name,"clients")==0) {
				Script::Collection(pState,1,"clients");
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "dump") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::Dump)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "joinGroup") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::JoinGroup)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "groups") == 0) {
				Script::Collection(pState, 1, "groups");
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "publications") == 0) {
				Script::Collection(pState, 1, "publications");
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "publish") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::Publish)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "toAMF") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::ToData<Mona::AMFWriter>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "toAMF0") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::ToAMF0)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "fromAMF") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::FromData<Mona::AMFReader>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "toJSON") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::ToData<Mona::JSONWriter>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "fromJSON") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::FromDataWithBuffers<Mona::JSONReader>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "toXMLRPC") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::ToData<Mona::XMLRPCWriter>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "fromXMLRPC") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::FromDataWithBuffers<Mona::XMLRPCReader>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "toXML") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::ToXML)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "fromXML") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::FromXML)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "toQuery") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::ToData<Mona::QueryWriter>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "fromQuery") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::FromData<Mona::QueryReader>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "absolutePath") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::AbsolutePath)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "time") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::Time)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "split") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::Split)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "createIPAddress") == 0) {
 				SCRIPT_WRITE_FUNCTION(LUAInvoker::CreateIPAddress<false>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "createIPAddressWithDNS") == 0) {
 				SCRIPT_WRITE_FUNCTION(LUAInvoker::CreateIPAddress<true>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "createSocketAddress") == 0) {
 				SCRIPT_WRITE_FUNCTION(LUAInvoker::CreateSocketAddress<false>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "createSocketAddressWithDNS") == 0) {
 				SCRIPT_WRITE_FUNCTION(LUAInvoker::CreateSocketAddress<true>)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "createUDPSocket") == 0) {
 				SCRIPT_WRITE_FUNCTION(LUAInvoker::CreateUDPSocket)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "createTCPClient") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::CreateTCPClient)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"createTCPServer")==0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::CreateTCPServer)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"md5")==0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::Md5)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"sha256")==0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::Sha256)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"addToBlacklist")==0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::AddToBlacklist)
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"removeFromBlacklist")==0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::RemoveFromBlacklist)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"configs")==0) {
				Script::Collection(pState,1, "configs");
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"environment")==0) {
				Script::Collection(pState,1, "environment");
				SCRIPT_FIX_RESULT
			} else if(strcmp(name,"servers")==0) {
				lua_getmetatable(pState, 1);
				lua_getfield(pState, -1, "|servers");
				lua_replace(pState, -2);
				SCRIPT_FIX_RESULT
			} else if (strcmp(name,"listPaths")==0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::ListFiles)
				SCRIPT_FIX_RESULT
			} else if (strcmp(name, "createMediaWriter") == 0) {
				SCRIPT_WRITE_FUNCTION(LUAInvoker::CreateMediaWriter)
				SCRIPT_FIX_RESULT
			} else {
				Script::Collection(pState,1, "configs");
				lua_getfield(pState, -1, name);
				if (lua_isnil(pState, -1) && strcmp(name, "arguments") == 0) {
					// to accept "mona.arguments" even if there is no arguments given on start (empty table so)
					lua_newtable(pState);
					lua_replace(pState, -2);
				}
				lua_replace(pState, -2);
				SCRIPT_FIX_RESULT
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAInvoker::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}

