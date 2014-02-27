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
#include "Mona/Exceptions.h"
#include "Mona/Files.h"
#include "MonaServer.h"
#include <openssl/evp.h>
#include "Mona/JSONReader.h"
#include "Mona/JSONWriter.h"
#include "Mona/XMLReader.h"
#include "Mona/XMLWriter.h"
#include "math.h"


using namespace std;
using namespace Mona;


void LUAInvoker::Init(lua_State *pState, Invoker& invoker) {
	// clients
	Script::Collection<Invoker,LUAClient>(pState, -1, "clients",0,&invoker);
	lua_pop(pState, 1);
	// publications
	Script::Collection<Invoker, LUAPublication<> >(pState, -1, "publications", 0, &invoker);
	lua_pop(pState, 1);
	// groups
	Script::Collection<Invoker, LUAGroup>(pState, -1, "groups", 0, &invoker);
	lua_pop(pState, 1);
}

void LUAInvoker::AddClient(lua_State *pState, Invoker& invoker, Client& client) {
	// -1 must be the client table!
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "clients", invoker.clients.count() + 1);
	LUAClient::GetID(pState, client);
	lua_pushvalue(pState, -4); // client table
	lua_rawset(pState, -3); // rawset cause NewIndexProhibited
	if (!client.name.empty()) {
		lua_pushstring(pState, client.name.c_str());
		lua_pushvalue(pState, -4);  // client table
		lua_rawset(pState, -3); // rawset cause NewIndexProhibited
	}
	lua_pop(pState, 2);
}

void LUAInvoker::RemoveClient(lua_State *pState, Invoker& invoker, const Client& client) {
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "clients", invoker.clients.count());
	LUAClient::GetID(pState, client);
	lua_pushnil(pState);
	lua_rawset(pState, -3); // rawset cause NewIndexProhibited
	if (!client.name.empty()) {
		lua_pushstring(pState, client.name.c_str());
		lua_pushnil(pState);
		lua_rawset(pState, -3); // rawset cause NewIndexProhibited
	}
	lua_pop(pState, 2);
}

void LUAInvoker::AddPublication(lua_State *pState, Invoker& invoker, const Publication& publication) {
	// -1 must be the publication table!
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "publications", invoker.publications.count());
	lua_pushstring(pState, publication.name().c_str());
	lua_pushvalue(pState, -4);
	lua_rawset(pState, -3); // rawset cause NewIndexProhibited
	lua_pop(pState, 2);
}

void LUAInvoker::RemovePublication(lua_State *pState, Invoker& invoker, const Publication& publication) {
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "publications", invoker.publications.count()-1);
	lua_pushstring(pState, publication.name().c_str());
	lua_pushnil(pState);
	lua_rawset(pState, -3); // rawset cause NewIndexProhibited
	lua_pop(pState, 2);
}

void LUAInvoker::AddGroup(lua_State *pState, Invoker& invoker, Group& group) {
	// -1 must be the group table!
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "groups", invoker.groups.count());
	lua_pushstring(pState,Util::FormatHex(group.id, ID_SIZE, invoker.buffer).c_str());
	lua_pushvalue(pState, -4);
	lua_rawset(pState, -3); // rawset cause NewIndexProhibited
	lua_pop(pState, 2);
}

void LUAInvoker::RemoveGroup(lua_State *pState, Invoker& invoker, const Group& group) {
	lua_getglobal(pState, "mona");
	Script::Collection(pState, -1, "groups", invoker.groups.count());
	lua_pushstring(pState,Util::FormatHex(group.id, ID_SIZE,invoker.buffer).c_str());
	lua_pushnil(pState);
	lua_rawset(pState, -3); // rawset cause NewIndexProhibited
	lua_pop(pState, 2);
}


int	LUAInvoker::Split(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		string expression = SCRIPT_READ_STRING("");
		string separator = SCRIPT_READ_STRING("");
		vector<string> values;
		String::Split(expression, separator, values,SCRIPT_READ_UINT(0));
		for(string& value : values)
			SCRIPT_WRITE_STRING(value.c_str())
	SCRIPT_CALLBACK_RETURN
}


int	LUAInvoker::Publish(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
	string name = SCRIPT_READ_STRING("");
	Exception ex;
	
	Publication* pPublication = invoker.publish(ex, name);

	if (!pPublication)
		SCRIPT_ERROR(ex ? ex.error().c_str() : "Unknown error")
	else {
		if (ex)
			SCRIPT_WARN(ex.error().c_str())
		SCRIPT_NEW_OBJECT(LUAMyPublication, LUAMyPublication, *new LUAMyPublication(*pPublication, invoker))
	}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::AbsolutePath(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		SCRIPT_WRITE_STRING((MonaServer::WWWPath + "/" + SCRIPT_READ_STRING("") + "/").c_str())
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::CreateUDPSocket(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		bool allowBroadcast = SCRIPT_READ_BOOL(false);
		SCRIPT_NEW_OBJECT(LUAUDPSocket,LUAUDPSocket,*(new LUAUDPSocket(invoker.sockets,allowBroadcast,pState)))
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::CreateTCPClient(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		SCRIPT_NEW_OBJECT(LUATCPClient, LUATCPClient, *(new LUATCPClient(invoker.sockets, pState)))
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::CreateTCPServer(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		SCRIPT_NEW_OBJECT(LUATCPServer, LUATCPServer, *(new LUATCPServer(invoker.sockets, pState)))
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::Md5(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		while(SCRIPT_CAN_READ) {
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

int LUAInvoker::Dir(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		Exception ex;
		string path(MonaServer::WWWPath + "/" + SCRIPT_READ_STRING("") + "/");
		Files dir(ex, path);
		for(auto itFile = dir.begin(); itFile != dir.end(); ++itFile) {
			SCRIPT_WRITE_STRING((*itFile).c_str());
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::Sha256(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		while(SCRIPT_CAN_READ) {
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
		writer.amf0Preference=true;
		SCRIPT_READ_DATA(writer)
		SCRIPT_WRITE_BINARY(writer.packet.data(),writer.packet.size())
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::AddToBlacklist(lua_State* pState) {
	SCRIPT_CALLBACK(Invoker,invoker)	
		while(SCRIPT_CAN_READ) {
			IPAddress address;
			Exception ex;
			bool success;
			EXCEPTION_TO_LOG(success=address.set(ex, SCRIPT_READ_STRING("")), "Blacklist entry")
			if (success)
				invoker.addBanned(address);
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::RemoveFromBlacklist(lua_State* pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		while(SCRIPT_CAN_READ) {
			IPAddress address;
			Exception ex;
			bool success;
			EXCEPTION_TO_LOG(success = address.set(ex, SCRIPT_READ_STRING("")), "Blacklist entry")
			if (success)
				invoker.removeBanned(address);
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAInvoker::JoinGroup(lua_State* pState) {
	SCRIPT_CALLBACK(Invoker, invoker)
		SCRIPT_READ_BINARY(peerId, size)
		if (size == (ID_SIZE << 1)) {
			invoker.buffer.assign((const char*)peerId,size);
			Util::UnformatHex(invoker.buffer);
		} else if (size != ID_SIZE) {
			if (peerId) {
				SCRIPT_ERROR("Bad member format id ", Util::FormatHex(peerId, size, invoker.buffer));
				peerId = NULL;
			} else
				SCRIPT_ERROR("Member id argument missing");
		}
		if (peerId) {
			SCRIPT_READ_BINARY(groupId, size)
			if (size == (ID_SIZE << 1)) {
				invoker.buffer.assign((const char*)groupId,size);
				Util::UnformatHex(invoker.buffer);
			} else if (size != ID_SIZE) {
				if (groupId) {
					SCRIPT_ERROR("Bad group format id ", Util::FormatHex(groupId, size, invoker.buffer))
					groupId = NULL;
				} else
					SCRIPT_ERROR("Group id argument missing")
			}
			if (groupId) {
				Peer* pPeer = new Peer((Handler&)invoker);
				memcpy((void*)pPeer->id, peerId, ID_SIZE);
				pPeer->joinGroup(groupId, NULL);
				SCRIPT_NEW_OBJECT(Peer, LUAMember, *pPeer)
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAInvoker::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		const char* name = SCRIPT_READ_STRING("");
		if(strcmp(name,"clients")==0) {
			Script::Collection(pState,1,"clients",invoker.clients.count());
		} else if (strcmp(name, "host") == 0) {
			SCRIPT_WRITE_STRING(((ServerHandler&)invoker).host().c_str())
		} else if (strcmp(name, "joinGroup") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::JoinGroup)
		} else if (strcmp(name, "groups") == 0) {
			Script::Collection(pState, 1, "groups", invoker.groups.count());
		} else if (strcmp(name, "publications") == 0) {
			Script::Collection(pState, 1, "publications", invoker.publications.count());
		} else if (strcmp(name, "publish") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::Publish)
		} else if (strcmp(name, "toAMF") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::ToData<Mona::AMFWriter>)
		} else if (strcmp(name, "toAMF0") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::ToAMF0)
		} else if (strcmp(name, "fromAMF") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::FromData<Mona::AMFReader>)
		} else if (strcmp(name, "toJSON") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::ToData<Mona::JSONWriter>)
		} else if (strcmp(name, "fromJSON") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::FromData<Mona::JSONReader>)
		} else if (strcmp(name, "toXML") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::ToData<Mona::XMLWriter>)
		} else if (strcmp(name, "fromXML") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::FromData<XMLReader>)
		} else if (strcmp(name, "absolutePath") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::AbsolutePath)
		} else if (strcmp(name, "epochTime") == 0) {
			SCRIPT_WRITE_NUMBER(Time::Now())
		} else if (strcmp(name, "split") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::Split)
		} else if (strcmp(name, "createUDPSocket") == 0) {
 			SCRIPT_WRITE_FUNCTION(&LUAInvoker::CreateUDPSocket)
		} else if (strcmp(name, "createTCPClient") == 0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::CreateTCPClient)
		} else if(strcmp(name,"createTCPServer")==0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::CreateTCPServer)
		} else if(strcmp(name,"md5")==0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::Md5)
		} else if(strcmp(name,"sha256")==0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::Sha256)
		} else if(strcmp(name,"addToBlacklist")==0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::AddToBlacklist)
		} else if(strcmp(name,"removeFromBlacklist")==0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::RemoveFromBlacklist)
		} else if (strcmp(name,"configs")==0) {
			lua_getmetatable(pState, LUA_GLOBALSINDEX);
			lua_getfield(pState, -1,"m.c");
			lua_replace(pState, -2);
		} else if (strcmp(name,"environments")==0) {
			lua_getmetatable(pState, LUA_GLOBALSINDEX);
			lua_getfield(pState, -1, "m.e");
			lua_replace(pState, -2);
		} else if(strcmp(name,"servers")==0) {
			lua_getglobal(pState, "m.s");
		} else if (strcmp(name,"dir")==0) {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::Dir)
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAInvoker::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Invoker,invoker)
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}

