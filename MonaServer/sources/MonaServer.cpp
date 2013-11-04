/*
	Copyright 2010 Mona - mathieu.poux[a]gmail.com

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

#include "MonaServer.h"
#include "Mona/Exceptions.h"
#include "Mona/FileSystem.h"
#include "LUAClient.h"
#include "LUAPublication.h"
#include "LUAPublications.h"
#include "LUAListener.h"
#include "LUAInvoker.h"
#include "LUAClients.h"
#include "LUAGroups.h"
#include "LUAGroup.h"
#include "LUAServer.h"
#include "LUAServers.h"
#include "LUABroadcaster.h"

#define CONFIG_NUMBER(NAME)  parameters.getNumber(#NAME,(double&)params.NAME);parameters.setNumber(#NAME,(double&)params.NAME);

#define CONFIG_PROTOCOL_NUMBER(PROTOCOL,NAME) parameters.getNumber(#PROTOCOL"."#NAME,(double&)params.PROTOCOL.NAME);parameters.setNumber(#PROTOCOL"."#NAME,(double&)params.PROTOCOL.NAME);if(#NAME=="port" && params.PROTOCOL.NAME>0) _ports[#PROTOCOL] = params.PROTOCOL.NAME;

using namespace std;
using namespace Mona;


const string MonaServer::WWWPath("./");

MonaServer::MonaServer(TerminateSignal& terminateSignal, UInt32 bufferSize, UInt32 threads, UInt16 serversPort, const string& serversTarget) :
Server(bufferSize, threads), servers(serversPort, *this, sockets, serversTarget), _terminateSignal(terminateSignal), _pService(NULL) {
}


bool MonaServer::start(MapParameters& parameters) {
	if(running()) {
		ERROR("Server is already running, call stop method before");
		return false;
	}
	parameters.getString("host", _host);

	string pathApp;
	parameters.getString("application.dir", pathApp);
	(string&)WWWPath = pathApp + "www";


	if (!FileSystem::CreateDirectory(WWWPath)) {
		ERROR("Impossible to create application directory ", WWWPath);
		return false;
	}

	_pState = Script::CreateState();
	Service::InitGlobalTable(_pState);

	ServerParams	params;

	// RTMFP
	parameters.getNumber("RTMFP.keepAliveServer",(double&)params.RTMFP.keepAliveServer);
	if (params.RTMFP.keepAliveServer < 5) {
		WARN("Value of RTMFP.keepAliveServer can't be less than 5 sec")
			parameters.setNumber("RTMFP.keepAliveServer", 5);
	}
	parameters.getNumber("RTMFP.keepAlivePeer", (double&)params.RTMFP.keepAlivePeer);
	if (params.RTMFP.keepAlivePeer < 5) {
		WARN("Value of RTMFP.keepAlivePeer can't be less than 5 sec")
			parameters.setNumber("RTMFP.keepAlivePeer", 5);
	}
	CONFIG_PROTOCOL_NUMBER(RTMFP, port);
	CONFIG_PROTOCOL_NUMBER(RTMFP, keepAliveServer);
	CONFIG_PROTOCOL_NUMBER(RTMFP, keepAlivePeer);

	// RTMP
	CONFIG_PROTOCOL_NUMBER(RTMP, port);

	// WebSocket
	CONFIG_PROTOCOL_NUMBER(HTTP, port);


	SCRIPT_BEGIN(_pState)
		SCRIPT_CREATE_PERSISTENT_OBJECT(Invoker,LUAInvoker,*this)
		readNextParameter(_pState, parameters, "");
		lua_setglobal(_pState,"mona.configs");
		readNextParameter(_pState, Util::Environment(), "");
		lua_setglobal(_pState, "mona.environments");
	SCRIPT_END

	return Server::start(params);
}

bool MonaServer::readNextParameter(lua_State* pState, const MapParameters& parameters, const string& root) {
	lua_newtable(pState);
	for (auto& it : parameters) {
		if (String::ICompare(it.second, "false") == 0 || String::ICompare(it.second, "nil") == 0)
			lua_pushboolean(_pState, 0);
		else
			lua_pushlstring(_pState, it.second.c_str(), it.second.size());
		lua_setfield(_pState,-2,it.first.c_str());
	}
	return true;
}



void MonaServer::addServiceFunction(Service& service, const std::string& name) {
	if(name=="onManage" || name=="onRendezVousUnknown" || name=="onServerConnection" || name=="onServerDisconnection")
		_scriptEvents[name].insert(&service);
}
void MonaServer::startService(Service& service) {
	_servicesRunning.insert(&service);
	// Send running information for all connected servers
	ServerMessage message;
	message.writer.writeString(service.path);
	servers.broadcast(".",message);
	
	SCRIPT_BEGIN(_pState)
		Servers::Iterator it;
		for(it=servers.begin();it!=servers.end();++it) {
			SCRIPT_FUNCTION_BEGIN("onServerConnection")
				SCRIPT_WRITE_PERSISTENT_OBJECT(ServerConnection,LUAServer,**it)
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		}
	SCRIPT_END
}
void MonaServer::stopService(Service& service) {
	_servicesRunning.erase(&service);

	// Call all the onServerDisconnection event for every services where service.path is running
	SCRIPT_BEGIN(_pState)
		Servers::Iterator it;
		for(it=servers.begin();it!=servers.end();++it) {
			SCRIPT_FUNCTION_BEGIN("onServerDisconnection")
				SCRIPT_WRITE_PERSISTENT_OBJECT(ServerConnection,LUAServer,**it)
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		}
	SCRIPT_END
}
void MonaServer::clearService(Service& service) {
	map<string,set<Service*> >::iterator it;
	for(it=_scriptEvents.begin();it!=_scriptEvents.end();++it)
		it->second.erase(&service);
}

void MonaServer::onStart() {
	_pService = new Service(_pState,"",*this);
	servers.start();
}
void MonaServer::onStop() {
	// delete service before servers.stop() to avoid a crash bug
	if(_pService) {
		delete _pService;
		_pService=NULL;
	}
	servers.stop();
	Script::CloseState(_pState);
	_ports.clear();
	_terminateSignal.set();
}


void MonaServer::manage() {
	Server::manage();
	_pService->refresh();
	set<Service*>& events = _scriptEvents["onManage"];
	set<Service*>::const_iterator it;
	for(it=events.begin();it!=events.end();++it) {
		SCRIPT_BEGIN((*it)->open())
			SCRIPT_FUNCTION_BEGIN("onManage")
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	}
	servers.manage();
}

void MonaServer::readLUAAddresses(const string& protocol,set<SocketAddress>& addresses) {
	lua_pushnil(_pState);  // first key 
	while (lua_next(_pState, -2) != 0) {
		// uses 'key' (at index -2) and 'value' (at index -1) 
		readLUAAddress(protocol,addresses);
		lua_pop(_pState,1);
	}
}


void MonaServer::readLUAAddress(const string& protocol,set<SocketAddress>& addresses) {
	int type = lua_type(_pState,-1);
	
	Exception ex;

	if(type==LUA_TTABLE) {
		bool isConst;
		Broadcaster* pBroadcaster = Script::ToObject<Broadcaster,LUAServers>(_pState,isConst);
		if(!pBroadcaster)
				pBroadcaster = Script::ToObject<Broadcaster,LUABroadcaster>(_pState,isConst);
		if(pBroadcaster) {
			for (ServerConnection* pServer : *pBroadcaster) {
				UInt16 port = pServer->port(protocol);
				if (port > 0) {
					IPAddress host;
					if (host.set(ex, pServer->host))
						addresses.emplace(host, port);
					else if (ex)
						goto EX;
				}
			}
			return;
		}
		ServerConnection* pServer = Script::ToObject<ServerConnection,LUAServer>(_pState,isConst);
		if(pServer) {
			UInt16 port = pServer->port(protocol);
			if (port > 0) {
				IPAddress host;
				if (host.set(ex, pServer->host))
					addresses.emplace(host, port);
				else if (ex)
					goto EX;
			}
			return;
		}
		readLUAAddresses(protocol,addresses);
	} else {
		const char* addr = lua_tostring(_pState,-1);
		if (addr) {
			if (String::ICompare(addr, "myself") == 0)
				addresses.emplace(); // wilcard, will be changed to the server public address by the caller
			else {
				SocketAddress address;
				if(address.set(ex,addr))
					addresses.emplace(address);
				else if (ex)
					goto EX;
			}
			
		}
	}
	return;
EX:
	SCRIPT_BEGIN(_pState)
		SCRIPT_ERROR("Invalid address, ",ex.error());
	SCRIPT_END
}


void MonaServer::onRendezVousUnknown(const string& protocol,const UInt8* id,set<SocketAddress>& addresses) {
	set<Service*>& events = _scriptEvents["onRendezVousUnknown"];
	set<Service*>::const_iterator it;
	for(it=events.begin();it!=events.end();++it) {
		SCRIPT_BEGIN((*it)->open())
			SCRIPT_FUNCTION_BEGIN("onRendezVousUnknown")
				SCRIPT_WRITE_STRING(protocol.c_str())
				SCRIPT_WRITE_BINARY(id,ID_SIZE)
				SCRIPT_FUNCTION_CALL
				while(SCRIPT_CAN_READ) {
					readLUAAddresses(protocol,addresses);
					SCRIPT_READ_NEXT
				}
			SCRIPT_FUNCTION_END
		SCRIPT_END
	}
}

void MonaServer::onHandshake(const string& protocol,const SocketAddress& address,const string& path,const map<string,string>& properties,UInt32 attempts,set<SocketAddress>& addresses) {
	Service* pService = _pService->get(path);
	if(!pService)
		return;
	SCRIPT_BEGIN(pService->open())
		SCRIPT_FUNCTION_BEGIN("onHandshake")
			SCRIPT_WRITE_STRING(protocol.c_str())
			SCRIPT_WRITE_STRING(address.toString().c_str())
			SCRIPT_WRITE_STRING(path.c_str())
			lua_newtable(_pState);
			map<string,string>::const_iterator it;
			for(it=properties.begin();it!=properties.end();++it) {
				lua_pushlstring(_pState,it->second.c_str(),it->second.size());
				lua_setfield(_pState,-2,it->first.c_str());
			}
			SCRIPT_WRITE_INT(attempts)
			SCRIPT_FUNCTION_CALL
			while(SCRIPT_CAN_READ) {
				readLUAAddress(protocol,addresses);
				SCRIPT_READ_NEXT
			}
		SCRIPT_FUNCTION_END
	SCRIPT_END
}


//// CLIENT_HANDLER /////
void MonaServer::onConnection(Exception& ex, Client& client,DataReader& parameters,DataWriter& response) {
	// Here you can read custom client http parameters in reading "client.parameters".
	Service* pService = _pService->get(client.path); 
	if(!pService) {
		ex.set(Exception::APPLICATION, "Applicaton ", client.path, " doesn't exist");
		return;
	}

	string error;
	SCRIPT_BEGIN(pService->open())
		SCRIPT_FUNCTION_BEGIN("onConnection")
			SCRIPT_WRITE_PERSISTENT_OBJECT(Client,LUAClient,client)
			SCRIPT_WRITE_DATA(parameters,0)
			SCRIPT_FUNCTION_CALL_WITHOUT_LOG
			if(SCRIPT_CAN_READ) {
				if(SCRIPT_NEXT_TYPE==LUA_TTABLE) {
					lua_pushnil(_pState);  // first key 
					while (lua_next(_pState, -2) != 0) {
						// uses 'key' (at index -2) and 'value' (at index -1) 
						// remove the raw!
						response.writePropertyName(lua_tostring(_pState,-2));
						Script::ReadData(_pState,response,1);
						lua_pop(_pState,1);
					}
				} else
					SCRIPT_ERROR("onConnection return argument ignored, it must be a table (with key=value)")
				SCRIPT_READ_NEXT
			}
		SCRIPT_FUNCTION_END
		if(SCRIPT_LAST_ERROR)
			error = strlen(SCRIPT_LAST_ERROR)>0 ? SCRIPT_LAST_ERROR : "client rejected";
		if(!pService)
			LUAClient::Clear(_pState,client);
	SCRIPT_END

	if(!error.empty()) {
		ex.set(Exception::SOFTWARE, error);
		return;
	}

	// build error!
	if(!pService->lastError.empty()) {
		ex.set(Exception::SOFTWARE, pService->lastError);
		return;
	}

	client.setNumber("&Service",(double)reinterpret_cast<unsigned>(pService));
	++pService->count;
}

// TODO keeping onFailed??
void MonaServer::onFailed(const Client& client,const string& error) {
	WARN("Client failed: ",error);
	double ptr = 0;
	if (!client.getNumber("&Service", ptr))
		return;
	Service* pService = reinterpret_cast<Service*>(static_cast<unsigned>(ptr));
	SCRIPT_BEGIN(pService->open())
		SCRIPT_FUNCTION_BEGIN("onFailed")
			SCRIPT_WRITE_PERSISTENT_OBJECT(Client,LUAClient,client)
			SCRIPT_WRITE_STRING(error.c_str())
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onDisconnection(const Client& client) {
	double ptr = 0;
	if (!client.getNumber("&Service", ptr))
		return;
	Service* pService = reinterpret_cast<Service*>(static_cast<unsigned>(ptr));
	SCRIPT_BEGIN(pService->open())
		SCRIPT_FUNCTION_BEGIN("onDisconnection")
			SCRIPT_WRITE_PERSISTENT_OBJECT(Client,LUAClient,client)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
	LUAClient::Clear(_pState,client);
	--pService->count;
}

void MonaServer::onMessage(Exception& ex, Client& client,const string& name,DataReader& reader) {
	string error("Method '" + name + "' not found");
	double ptr = 0;
	if (!client.getNumber("&Service", ptr)) {
		ex.set(Exception::APPLICATION, error);
		return;
	}
	Service* pService = reinterpret_cast<Service*>(static_cast<unsigned>(ptr));
	SCRIPT_BEGIN(pService->open())
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client,LUAClient,client,name.c_str())
			SCRIPT_WRITE_DATA(reader,0)
			error.clear();
			SCRIPT_FUNCTION_CALL_WITHOUT_LOG
			if(SCRIPT_CAN_READ) {
				Script::ReadData(_pState,client.writer().writeMessage(),1);
				++__args;
			}
		SCRIPT_FUNCTION_END
		if(SCRIPT_LAST_ERROR) {
			ex.set(Exception::APPLICATION,SCRIPT_LAST_ERROR);
			return;
		}
	SCRIPT_END
	if(!error.empty()) {
		ERROR(error);
		ex.set(Exception::APPLICATION, error);
		return;
	}
}

bool MonaServer::onRead(Exception& ex, Client& client,string& filePath,DataReader& parameters) { 
	bool result = true;
	double ptr = 0;
	if (!client.getNumber("&Service", ptr))
		return result;
	Service* pService = reinterpret_cast<Service*>(static_cast<unsigned>(ptr));
	filePath = WWWPath + filePath;
	SCRIPT_BEGIN(pService->open())
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client,LUAClient,client,"onRead")
			SCRIPT_WRITE_STRING(filePath.c_str())
			SCRIPT_WRITE_DATA(parameters,0)
			SCRIPT_FUNCTION_CALL
			if(SCRIPT_CAN_READ) {
				if(SCRIPT_NEXT_TYPE==LUA_TNIL)
					result=false;
				else
					filePath = SCRIPT_READ_STRING(filePath);
			}
		SCRIPT_FUNCTION_END
		if (SCRIPT_LAST_ERROR) {
			ex.set(Exception::SOFTWARE, SCRIPT_LAST_ERROR);
			return false;
		}
	SCRIPT_END
	return result;
}


//// PUBLICATION_HANDLER /////
bool MonaServer::onPublish(Client& client,const Publication& publication,string& error) {
	if(client == this->id)
		return true;
	bool result=true;
	double ptr = 0;
	if (!client.getNumber("&Service", ptr))
		return result;
	Service* pService = reinterpret_cast<Service*>(static_cast<unsigned>(ptr));
	SCRIPT_BEGIN(pService->open())
		SCRIPT_FUNCTION_BEGIN("onPublish")
			SCRIPT_WRITE_PERSISTENT_OBJECT(Client,LUAClient,client)
			SCRIPT_WRITE_PERSISTENT_OBJECT(Publication,LUAPublication,publication)
			SCRIPT_FUNCTION_CALL
			if(SCRIPT_CAN_READ)
				result = SCRIPT_READ_BOOL(true);
		SCRIPT_FUNCTION_END
		if(SCRIPT_LAST_ERROR) {
			error = SCRIPT_LAST_ERROR;
			result = false;
		}
		if(!result)
			LUAPublication::Clear(_pState,publication);
	SCRIPT_END
	return result;
}

void MonaServer::onUnpublish(Client& client,const Publication& publication) {
	double ptr = 0;
	if (client.getNumber("&Service", ptr))
		return;
	if(client != this->id) {
		Service* pService = reinterpret_cast<Service*>(static_cast<unsigned>(ptr));
		SCRIPT_BEGIN(pService->open())
			SCRIPT_FUNCTION_BEGIN("onUnpublish")
				SCRIPT_WRITE_PERSISTENT_OBJECT(Client, LUAClient, client)
				SCRIPT_WRITE_PERSISTENT_OBJECT(Publication, LUAPublication, publication)
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	}
	if(publication.listeners.count()==0)
		LUAPublication::Clear(_pState,publication);
}

bool MonaServer::onSubscribe(Client& client,const Listener& listener,string& error) { 
	bool result=true;
	double ptr = 0;
	if (!client.getNumber("&Service", ptr))
		return result;
	Service* pService = reinterpret_cast<Service*>(static_cast<unsigned>(ptr));
	SCRIPT_BEGIN(pService->open())
		SCRIPT_FUNCTION_BEGIN("onSubscribe")
			SCRIPT_WRITE_PERSISTENT_OBJECT(Client,LUAClient,client)
			SCRIPT_WRITE_PERSISTENT_OBJECT(Listener,LUAListener,listener)
			SCRIPT_FUNCTION_CALL
			if(SCRIPT_CAN_READ)
				result = SCRIPT_READ_BOOL(true);
		SCRIPT_FUNCTION_END
		if(SCRIPT_LAST_ERROR) {
			error = SCRIPT_LAST_ERROR;
			result = false;
		}
		if(!result)
			LUAListener::Clear(_pState,listener);
	SCRIPT_END
	return result;
}

void MonaServer::onUnsubscribe(Client& client,const Listener& listener) {
	double ptr = 0;
	if (!client.getNumber("&Service", ptr))
		return;
	Service* pService = reinterpret_cast<Service*>(static_cast<unsigned>(ptr));
	SCRIPT_BEGIN(pService->open())
		SCRIPT_FUNCTION_BEGIN("onUnsubscribe")
			SCRIPT_WRITE_PERSISTENT_OBJECT(Client,LUAClient,client)
			SCRIPT_WRITE_PERSISTENT_OBJECT(Listener,LUAListener,listener)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
	if(!listener.publication.publisher() && listener.publication.listeners.count()==0)
		LUAPublication::Clear(_pState,listener.publication);
	LUAListener::Clear(_pState,listener);
}

void MonaServer::onAudioPacket(Client& client,const Publication& publication,UInt32 time,MemoryReader& packet) {
	if(client == this->id)
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication,LUAPublication,publication,"onAudio")
			SCRIPT_WRITE_NUMBER(time)
			SCRIPT_WRITE_BINARY(packet.current(),packet.available())
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onVideoPacket(Client& client,const Publication& publication,UInt32 time,MemoryReader& packet) {
	if(client == this->id)
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication,LUAPublication,publication,"onVideo")
			SCRIPT_WRITE_NUMBER(time)
			SCRIPT_WRITE_BINARY(packet.current(),packet.available())
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onDataPacket(Client& client,const Publication& publication,DataReader& packet) {
	if(client == this->id)
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication,LUAPublication,publication,"onData")
			SCRIPT_WRITE_DATA(packet,0)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onFlushPackets(Client& client,const Publication& publication) {
	if(client == this->id)
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication,LUAPublication,publication,"onFlush")
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}


void MonaServer::onJoinGroup(Client& client,Group& group) {
	double ptr = 0;
	if (!client.getNumber("&Service", ptr))
		return;
	Service* pService = reinterpret_cast<Service*>(static_cast<unsigned>(ptr));
	SCRIPT_BEGIN(pService->open())
		SCRIPT_FUNCTION_BEGIN("onJoinGroup")
			SCRIPT_WRITE_PERSISTENT_OBJECT(Client,LUAClient,client)
			SCRIPT_WRITE_PERSISTENT_OBJECT(Group,LUAGroup,group)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onUnjoinGroup(Client& client,Group& group) {
	double ptr = 0;
	if (!client.getNumber("&Service", ptr))
		return;
	Service* pService = reinterpret_cast<Service*>(static_cast<unsigned>(ptr));
	SCRIPT_BEGIN(pService->open())
		SCRIPT_FUNCTION_BEGIN("onUnjoinGroup")
			SCRIPT_WRITE_PERSISTENT_OBJECT(Client,LUAClient,client)
			SCRIPT_WRITE_PERSISTENT_OBJECT(Group,LUAGroup,group)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
	if(group.size()==0)
		Script::ClearPersistentObject<Group,LUAGroup>(_pState,group);
}

void MonaServer::onManage(Client& client) {
	double ptr = 0;
	if (!client.getNumber("&Service", ptr))
		return;
	Service* pService = reinterpret_cast<Service*>(static_cast<unsigned>(ptr));
	SCRIPT_BEGIN(pService->open())
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client,LUAClient,client,"onManage")
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}


void MonaServer::connection(ServerConnection& server) {
	// sends actual services online to every connected servers
	ServerMessage message;
	for(const Service* pService : _servicesRunning)
		message.writer.writeString(pService->path);
	servers.broadcast(".",message);

	set<Service*>& events = _scriptEvents["onServerConnection"];
	set<Service*>::const_iterator it2;
	for(it2=events.begin();it2!=events.end();++it2) {
		SCRIPT_BEGIN((*it2)->open())
			SCRIPT_FUNCTION_BEGIN("onServerConnection")
				SCRIPT_WRITE_PERSISTENT_OBJECT(ServerConnection,LUAServer,server)
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	}
}

void MonaServer::message(ServerConnection& server,const std::string& handler,MemoryReader& reader) {
	if(handler==".") {
		while(reader.available()) {
			string path;
			reader.readString(path);
			_pService->get(path);
		}
		return;
	}
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(ServerConnection,LUAServer,server,handler.c_str())
			AMFReader amf(reader);
			SCRIPT_WRITE_DATA(amf,0)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::disconnection(const ServerConnection& server,const string& error) {
	set<Service*>& events = _scriptEvents["onServerDisconnection"];
	set<Service*>::const_iterator it;
	for(it=events.begin();it!=events.end();++it) {
		SCRIPT_BEGIN((*it)->open())
			SCRIPT_FUNCTION_BEGIN("onServerDisconnection")
				SCRIPT_WRITE_PERSISTENT_OBJECT(ServerConnection,LUAServer,server)
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	}

	LUAServer::Clear(_pState,server);
}

