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

#include "LUAInvoker.h"
#include "LUAClients.h"
#include "LUAPublication.h"
#include "LUAPublications.h"
#include "LUAGroups.h"
#include "LUAUDPSocket.h"
#include "LUATCPClient.h"
#include "LUATCPServer.h"
#include "LUAServers.h"
#include "MonaServer.h"
#include <openssl/evp.h>
#include "math.h"

using namespace Poco;
using namespace Poco::Net;
using namespace std;

const char*		LUAInvoker::Name="Mona::Invoker";


int	LUAInvoker::Split(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		string expression = SCRIPT_READ_STRING("");
		string separator = SCRIPT_READ_STRING("");
		StringTokenizer split(expression,separator,SCRIPT_READ_UINT(0));
		StringTokenizer::Iterator it;
		for(it=split.begin();it!=split.end();++it)
			SCRIPT_WRITE_STRING(it->c_str())
	SCRIPT_CALLBACK_RETURN
}


int	LUAInvoker::Publish(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		string name = SCRIPT_READ_STRING("");
		try {
			SCRIPT_WRITE_PERSISTENT_OBJECT(Mona::Publication,LUAPublication,invoker.publish(name))
			lua_getmetatable(pState,-1);
			lua_pushlightuserdata(pState,&invoker);
			lua_setfield(pState,-2,"__invoker");
			lua_pop(pState,1);
		} catch(Exception& ex) {
			SCRIPT_ERROR("%s",ex.message().c_str())
			SCRIPT_WRITE_NIL
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::AbsolutePath(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		SCRIPT_WRITE_STRING((MonaServer::WWWPath + SCRIPT_READ_STRING("") + "/").c_str())
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::CreateUDPSocket(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		bool allowBroadcast = SCRIPT_READ_BOOL(false);
		SCRIPT_WRITE_OBJECT(LUAUDPSocket,LUAUDPSocket,*(new LUAUDPSocket(invoker.sockets,allowBroadcast,pState)))
		SCRIPT_ADD_DESTRUCTOR(&LUAUDPSocket::Destroy)
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::CreateTCPClient(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		SCRIPT_WRITE_OBJECT(LUATCPClient,LUATCPClient,*(new LUATCPClient(invoker.sockets,pState)))
		SCRIPT_ADD_DESTRUCTOR(&LUATCPClient::Destroy);
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::CreateTCPServer(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		SCRIPT_WRITE_OBJECT(LUATCPServer,LUATCPServer,*(new LUATCPServer(invoker.sockets,pState)))
		SCRIPT_ADD_DESTRUCTOR(&LUATCPServer::Destroy);
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::Md5(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		while(SCRIPT_CAN_READ) {
			SCRIPT_READ_BINARY(data,size)
			if(data) {
				UInt8 result[32];
				EVP_Digest(data,size,(unsigned char *)result,NULL,EVP_md5(),NULL);
				SCRIPT_WRITE_BINARY(result,32);
			} else {
				SCRIPT_ERROR("Input MD5 value have to be a string expression")
				SCRIPT_WRITE_NIL
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::Sha256(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		while(SCRIPT_CAN_READ) {
			SCRIPT_READ_BINARY(data,size)
			if(data) {
				UInt8 result[32];
				EVP_Digest(data,size,(unsigned char *)result,NULL,EVP_sha256(),NULL);
				SCRIPT_WRITE_BINARY(result,32);
			} else {
				SCRIPT_ERROR("Input SHA256 value have to be a string expression")
				SCRIPT_WRITE_NIL
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::ToAMF(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		Mona::AMFWriter writer;
		SCRIPT_READ_DATA(writer)
		SCRIPT_WRITE_BINARY(writer.stream.data(),writer.stream.size())
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::ToAMF0(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		Mona::AMFWriter writer;
		writer.amf0Preference=true;
		SCRIPT_READ_DATA(writer)
		SCRIPT_WRITE_BINARY(writer.stream.data(),writer.stream.size())
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::FromAMF(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		SCRIPT_READ_BINARY(data,size)
		Mona::MemoryReader packet(data,size);
		Mona::AMFReader reader(packet);
		SCRIPT_WRITE_DATA(reader,SCRIPT_READ_UINT(0))
	SCRIPT_CALLBACK_RETURN
}


int	LUAInvoker::AddToBlacklist(lua_State* pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)	
		while(SCRIPT_CAN_READ) {
			try {		
				invoker.addBanned(IPAddress(SCRIPT_READ_STRING("")));
			} catch(Exception& ex) {
				SCRIPT_ERROR("Incomprehensible blacklist entry, %s",ex.displayText().c_str());
			}
		}
	SCRIPT_CALLBACK_RETURN
}

int	LUAInvoker::RemoveFromBlacklist(lua_State* pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		while(SCRIPT_CAN_READ) {
			try {
				invoker.removeBanned(IPAddress(SCRIPT_READ_STRING("")));
			} catch(Exception& ex) {
				SCRIPT_ERROR("Incomprehensible blacklist entry, %s",ex.displayText().c_str());
			}
		}
	SCRIPT_CALLBACK_RETURN
}


int LUAInvoker::Get(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		string name = SCRIPT_READ_STRING("");
		if(name=="clients") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(Mona::Clients,LUAClients,invoker.clients)
		} else if(name=="host") {
			SCRIPT_WRITE_STRING(((ServerHandler&)invoker).host().c_str())
		} else if(name=="groups") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(Mona::Entities<Mona::Group>,LUAGroups,invoker.groups)
		} else if(name=="publications") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(Mona::Publications,LUAPublications,invoker.publications)
		} else if(name=="servers") {
			SCRIPT_WRITE_PERSISTENT_OBJECT(Servers,LUAServers,((MonaServer&)invoker).servers)
		} else if(name=="publish") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::Publish)
		} else if(name=="toAMF") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::ToAMF)
		} else if(name=="toAMF0") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::ToAMF0)
		} else if(name=="fromAMF") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::FromAMF)
		} else if(name=="absolutePath") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::AbsolutePath)
		} else if(name=="epochTime") {
			SCRIPT_WRITE_NUMBER(ROUND(Mona::Time().toInt()/1000))
		} else if(name=="split") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::Split)
		} else if(name=="createUDPSocket") {
 			SCRIPT_WRITE_FUNCTION(&LUAInvoker::CreateUDPSocket)
		} else if(name=="createTCPClient") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::CreateTCPClient)
		} else if(name=="createTCPServer") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::CreateTCPServer)
		} else if(name=="configs") {
			lua_getglobal(pState,"mona.configs");
		} else if(name=="md5") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::Md5)
		} else if(name=="sha256") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::Sha256)
		} else if(name=="addToBlacklist") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::AddToBlacklist)
		} else if(name=="removeFromBlacklist") {
			SCRIPT_WRITE_FUNCTION(&LUAInvoker::RemoveFromBlacklist)
		}
	SCRIPT_CALLBACK_RETURN
}

int LUAInvoker::Set(lua_State *pState) {
	SCRIPT_CALLBACK(Mona::Invoker,LUAInvoker,invoker)
		string name = SCRIPT_READ_STRING("");
		lua_rawset(pState,1); // consumes key and value
	SCRIPT_CALLBACK_RETURN
}

