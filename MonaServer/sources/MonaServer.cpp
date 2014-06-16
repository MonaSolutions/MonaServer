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

#include "MonaServer.h"
#include "LUAClient.h"
#include "LUAPublication.h"
#include "LUAListener.h"
#include "LUAInvoker.h"
#include "LUAGroup.h"
#include "LUAServer.h"
#include "LUABroadcaster.h"
#include "LUADataTable.h"


using namespace std;
using namespace Mona;


const string MonaServer::WWWPath("./");
const string MonaServer::DataPath("./");


MonaServer::MonaServer(TerminateSignal& terminateSignal, const Parameters& configs) : _pState(NULL),
	Server(configs.getNumber<UInt32>("socketBufferSize"), configs.getNumber<UInt16>("threads")), servers(sockets), _firstData(true),_data(this->poolBuffers),_terminateSignal(terminateSignal),
	setLUAProperty([this](const string& key, const string& value) { Script::PushValue(_pState, value); lua_setfield(_pState, -2, key.c_str());} ) {
	
	Parameters::ForEach forEach([this](const string& key, const string& value) {
		setString(key, value);
	});
	configs.iterate(forEach);

	string pathApp;
	configs.getString("application.dir", pathApp);
	(string&)WWWPath = pathApp + "www";
	(string&)DataPath = pathApp + "data";


	onServerConnection = [this](ServerConnection& server) {
		Script::AddObject<LUAServer>(_pState, server);
		LUABroadcaster::AddServer(_pState,servers, server.address.toString());
		LUABroadcaster::AddServer(_pState, server.isTarget ? servers.targets : servers.initiators, server.address.toString());

		Exception ex;
		if (!_pService->open(ex)) {
			server.reject(ex.error().c_str());
			return;
		}
		
		bool error(false);
		SCRIPT_BEGIN(_pState)
			SCRIPT_FUNCTION_BEGIN("onServerConnection",_pService->reference())
				lua_pushvalue(_pState, 1); // server! (see Script::AddObject above)
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
			if (SCRIPT_LAST_ERROR) {
				error = true;
				server.reject(SCRIPT_LAST_ERROR);
			}
		SCRIPT_END
		lua_pop(_pState, 1); // remove Script::AddObject<ServerConnection,... (see above)
		if (error)
			return;
		// sends actual services online to the new server connected
		shared_ptr<ServerMessage> pMessage(new ServerMessage(".",poolBuffers));
		for(const Service* pService : _servicesRunning)
			pMessage->packet.writeString(pService->path);
		server.send(pMessage);
	};

	onServerMessage = [this](ServerConnection& server, const string& handler, PacketReader& packet) {
		if (handler == ".") {
			while (packet.available()) {
				string path;
				packet.readString(path);
				// load the service relating with remoting server
				Exception ex;
				_pService->open(ex,path);
			}
			return;
		}
		SCRIPT_BEGIN(_pState)
			SCRIPT_MEMBER_FUNCTION_BEGIN(ServerConnection, server, handler.c_str());
				AMFReader amf(packet);
				SCRIPT_WRITE_DATA(amf, 0);
				SCRIPT_FUNCTION_CALL;
			SCRIPT_FUNCTION_END;
		SCRIPT_END;
	};

	onServerDisconnection = [this](const Exception& ex,const ServerConnection& server) {
		Exception exc;
		if (_pService->open(exc)) {
			SCRIPT_BEGIN(_pState)
				SCRIPT_FUNCTION_BEGIN("onServerDisconnection",_pService->reference())
					Script::AddObject<LUAServer>(_pState, server);
					if (ex)
						SCRIPT_WRITE_STRING(ex.error().c_str())
					SCRIPT_FUNCTION_CALL
				SCRIPT_FUNCTION_END
			SCRIPT_END
		}
		LUABroadcaster::RemoveServer(_pState, servers, server.address.toString());
		LUABroadcaster::RemoveServer(_pState, server.isTarget ? servers.targets : servers.initiators, server.address.toString());
		Script::ClearObject<LUAServer>(_pState, server);
	};


	servers.OnConnection::subscribe(onServerConnection);
	servers.OnMessage::subscribe(onServerMessage);
	servers.OnDisconnection::subscribe(onServerDisconnection);
}

MonaServer::~MonaServer() {
	stop();
	servers.OnConnection::unsubscribe(onServerConnection);
	servers.OnMessage::unsubscribe(onServerMessage);
	servers.OnDisconnection::unsubscribe(onServerDisconnection);
}


bool MonaServer::start() {
	if(Server::running()) {
		ERROR("Server is already running, call stop method before");
		return false;
	}

	if (!FileSystem::CreateDirectory(WWWPath)) {
		ERROR("Impossible to create application directory ", WWWPath);
		return false;
	}

	return Server::start();
}

void MonaServer::startService(Service& service) {
	_servicesRunning.insert(&service);

	// Send running information for all connected servers (to refresh service related)
	PacketWriter writer(poolBuffers);
	writer.writeString(service.path);
	servers.broadcast(".",writer);
	
	SCRIPT_BEGIN(_pState)
		for(ServerConnection* pServer : servers) {
			SCRIPT_FUNCTION_BEGIN("onServerConnection",_pService->reference())
				Script::AddObject<LUAServer>(_pState, *pServer);
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
			if (SCRIPT_LAST_ERROR)
				pServer->reject(SCRIPT_LAST_ERROR);
		}
	SCRIPT_END

}
void MonaServer::stopService(Service& service) {
	_servicesRunning.erase(&service);

	// Call all the onServerDisconnection event for every services where service.path is running
	SCRIPT_BEGIN(_pState)
		for(ServerConnection* pServer : servers) {
			SCRIPT_FUNCTION_BEGIN("onServerDisconnection",_pService->reference())
				Script::AddObject<LUAServer>(_pState, *pServer);
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		}
	SCRIPT_END
}

void MonaServer::onStart() {
	
	_pState = Script::CreateState();


	// init root server application
	_pService.reset(new Service(_pState, *this));

	// Init few global variable
	Script::AddObject<LUAInvoker,Invoker>(_pState,*this);
	lua_getmetatable(_pState, -1);

	Script::AddObject<LUABroadcaster>(_pState, servers);
	lua_getmetatable(_pState, -1);	

	Script::AddObject<LUABroadcaster>(_pState, servers.initiators);
	lua_setfield(_pState, -2, "|initiators");
	Script::AddObject<LUABroadcaster>(_pState, servers.targets);
	lua_setfield(_pState, -2, "|targets");
	lua_pop(_pState, 1);

	lua_pushvalue(_pState, -1);
	lua_setfield(_pState, -3, "|servers");
	lua_setfield(_pState, -3, "servers");
	lua_pop(_pState, 1); // remove metatable

	lua_setglobal(_pState,"mona");

	Script::NewObject<LUADataTable>(_pState, *new LUADataTable(_data, _pService->path));
	lua_setfield(_pState, LUA_REGISTRYINDEX, "|data");
	
	// load database
	Exception ex;
	_firstData = true;
	if (_data.load(ex, DataPath, *this, true)) {
		if (ex)
			ERROR("Error on database loading, ", ex.error())
		NOTE("Database loaded")
	}
	
	// start servers
	servers.start(*this);

	// start the application (if exists)
	ex.set(Exception::NIL);
	_pService->open(ex);
}


void MonaServer::onDataLoading(const string& path, const UInt8* value, UInt32 size) {

	if (_firstData) {
		INFO("Database loading...")
		_firstData = false;
	}
	if (!_pState) {
		ERROR("Loading database useless, no recipient")
		return;
	}
	// get table and set key
	lua_pushvalue(_pState,LUA_REGISTRYINDEX);
	string key("|data");
	if (!path.empty()) {
		lua_getfield(_pState, LUA_REGISTRYINDEX, "|data");
		lua_replace(_pState, -2);
		if (lua_isnil(_pState, -1)) {
			lua_pop(_pState, 1);
			ERROR("Loading database impossible because no recipient")
			return;
		}
	}
	vector<string> fields;
	String::Split(path, "/", fields, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
	if (!fields.empty()) {
		key = move(fields.back());
		fields.pop_back();
	}
	for (const string& field : fields) {
		if (!lua_istable(_pState, -1)) {
			lua_pop(_pState, 1);
			WARN("Loading database entry ", path, " ignored because parent is not a table")
				return;
		}
		lua_getfield(_pState, -1, field.c_str());
		if (lua_isnil(_pState, -1)) {
			lua_pop(_pState, 1);
			lua_newtable(_pState);
			lua_setfield(_pState, -2, field.c_str());
			lua_getfield(_pState, -1, field.c_str());
		}
		lua_replace(_pState, -2);
	}

	// set value
	Script::PushValue(_pState,value,size);
	lua_setfield(_pState, -2, key.c_str());
	lua_pop(_pState, 1); // remove table
}


void MonaServer::onStop() {
	// disconnect before "servers.stop" to try to get a gracefull tcp close
	for (ServerConnection* pServer : servers)
		pServer->reject("Server is stopping");
	// delete service before servers.stop() to avoid a crash bug
	if(_pService)
		_pService.reset();
	Script::CloseState(_pState);
	_pState = NULL;
	if (_data.writing()) {
		INFO("Database flushing...")
		_data.flush();
	}
	servers.stop();
	_terminateSignal.set();
}


void MonaServer::manage() {
	Server::manage();
	servers.manage(*this);
	if (!_pService)
		return;

	// control script size to debug!
	if (lua_gettop(_pState) != 0)
		CRITIC("LUA stack corrupted, contains ",lua_gettop(_pState)," irregular values");

	Exception ex;
	if (!_pService->open(ex))
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_FUNCTION_BEGIN("onManage",_pService->reference())
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::readLUAAddresses(const string& protocol,set<SocketAddress>& addresses) {
	lua_pushnil(_pState);  // first key
	while (lua_next(_pState, -2) != 0) {
		// uses 'key' (at index -2) and 'value' (at index -1) 
		readLUAAddress(protocol,addresses);
		lua_pop(_pState,1);
	}
}


void MonaServer::readLUAAddress(const string& protocol, set<SocketAddress>& addresses) {
	int type = lua_type(_pState, -1);

	Exception ex;

	if (type == LUA_TTABLE) {
		bool isConst;
		Broadcaster* pBroadcaster = Script::ToObject<Broadcaster>(_pState, isConst);
		if (pBroadcaster) {
			for (ServerConnection* pServer : *pBroadcaster)
				readLUAServerAddress(protocol, addresses, *pServer);
		} else {
			ServerConnection* pServer = Script::ToObject<ServerConnection>(_pState, isConst);
			if (pServer)
				readLUAServerAddress(protocol, addresses, *pServer);
			else
				readLUAAddresses(protocol, addresses);
		}
	} else {
		const char* addr = lua_tostring(_pState, -1);
		if (addr) {
			if (String::ICompare(addr, "myself") == 0)
				addresses.emplace(); // wilcard, will be changed to the server public address by the caller
			else
				addLUAAddress(addresses, addr);
		}
	}
}

void MonaServer::readLUAServerAddress(const string& protocol, set<SocketAddress>& addresses,const ServerConnection& server) {
	UInt16 port(0);
	string buffer;
	if (!server.getNumber(String::Format(buffer, protocol, ".port"), port)) {
		SCRIPT_BEGIN(_pState)
			SCRIPT_ERROR("Impossible to determine ", protocol, " port of ", server.address.toString(), " server");
		SCRIPT_END
		return;
	}
	if (port == 0) {
		SCRIPT_BEGIN(_pState)
			SCRIPT_WARN("Server ",server.address.toString()," has ",protocol," disabled");
		SCRIPT_END
		return;
	}

	if (!server.getString(String::Format(buffer, protocol, ".publicHost"), buffer) && !server.getString("publicHost", buffer))
		buffer = server.address.host().toString();

	addLUAAddress(addresses,buffer, port);
}


void MonaServer::onRendezVousUnknown(const string& protocol,const UInt8* id,set<SocketAddress>& addresses) {
	Exception ex;
	if (!_pService->open(ex))
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_FUNCTION_BEGIN("onRendezVousUnknown",_pService->reference())
			SCRIPT_WRITE_STRING(protocol.c_str())
			SCRIPT_WRITE_BINARY(id,ID_SIZE)
			SCRIPT_FUNCTION_CALL
			while(SCRIPT_CAN_READ) {
				readLUAAddress(protocol,addresses);
				SCRIPT_READ_NEXT
			}
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onHandshake(const string& protocol,const SocketAddress& address,const string& path,const Parameters& properties,UInt32 attempts,set<SocketAddress>& addresses) {
	Exception ex;
	Service* pService(_pService->open(ex, path));
	if (!pService)
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_FUNCTION_BEGIN("onHandshake",pService->reference())
			SCRIPT_WRITE_STRING(protocol.c_str())
			SCRIPT_WRITE_STRING(address.toString().c_str())
			SCRIPT_WRITE_STRING(path.c_str())
			lua_newtable(_pState);
			properties.iterate(setLUAProperty);
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
	Service* pService = _pService->open(ex,client.path);
	if (!pService) {
		if (ex)
			ERROR(ex.error())
		return;
	}

	Script::AddObject<LUAClient>(_pState,client);

	SCRIPT_BEGIN(_pState)
		SCRIPT_FUNCTION_BEGIN("onConnection",pService->reference())
			lua_pushvalue(_pState, 1); // client! (see Script::AddObject above)
			SCRIPT_WRITE_DATA(parameters,0)
			SCRIPT_FUNCTION_CALL
			if(SCRIPT_CAN_READ) {
				if(SCRIPT_NEXT_TYPE==LUA_TTABLE) {
					lua_pushnil(_pState);  // first key 
					while (lua_next(_pState, -2) != 0) {
						// uses 'key' (at index -2) and 'value' (at index -1) 
						if (lua_isstring(_pState, -2)) {
							response.writePropertyName(lua_tostring(_pState,-2));
							Script::ReadData(_pState,response,1);
						} else
							SCRIPT_WARN("key=value ignored because key is not a string value")
						lua_pop(_pState,1);
					}
					response.endWrite(); // TODO remove! the caller can continue to write the response!
					SCRIPT_READ_NEXT
				} else
					SCRIPT_ERROR("onConnection return argument ignored, it must be a table {key1=value1,key2=value2}")
			}
		SCRIPT_FUNCTION_END
		if(SCRIPT_LAST_ERROR)
			ex.set(Exception::SOFTWARE, strlen(SCRIPT_LAST_ERROR)>0 ? SCRIPT_LAST_ERROR : "client rejected");
	SCRIPT_END

	if (ex) {
		Script::ClearObject<LUAClient>(_pState,client);
		lua_pop(_pState, 1); // remove Script::AddObject<Client .. (see above)
		return;
	}

	LUAInvoker::AddClient(_pState);
	// connection accepted
	openService(*pService,client);
	lua_pop(_pState, 1); // remove Script::AddObject<Client .. (see above)	
}

lua_State* MonaServer::openService(const Service& service, Client& client) {
	// -1 must be client table
	if (!_pState || service.reference() == LUA_REFNIL)
		return NULL;
	lua_rawgeti(_pState, LUA_REGISTRYINDEX, (int)client.data(service.reference()));
	Script::Collection(_pState, -1, "clients");
	lua_getfield(_pState, -3,"id");
	lua_pushvalue(_pState, -4);
	Script::FillCollection(_pState, 1);
	lua_pop(_pState, 2);
	return _pState;
}

lua_State* MonaServer::closeService(const Client& client) {
	// -1 must be client table
	if (!_pState || client.data() == LUA_REFNIL)
		return NULL;
	lua_rawgeti(_pState, LUA_REGISTRYINDEX, (int)client.data());
	Script::Collection(_pState, -1, "clients");
	lua_getfield(_pState, -3,"id");
	lua_pushnil(_pState);
	Script::FillCollection(_pState, 1);
	lua_pop(_pState, 2);
	return _pState;
}

void MonaServer::onDisconnection(const Client& client) {
	Script::AddObject<LUAClient>(_pState, client);
	LUAInvoker::RemoveClient(_pState);
	
	SCRIPT_BEGIN(closeService(client))
		SCRIPT_FUNCTION_BEGIN("onDisconnection",(int)client.data())
			lua_pushvalue(_pState, 1); // client! (see Script::AddObject above)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END

	lua_pop(_pState, 1);  // remove Script::AddObject<Client .. (see above)

	Script::ClearObject<LUAClient>(_pState,client);
}

bool MonaServer::onMessage(Exception& ex, Client& client,const string& name,DataReader& reader, UInt8 responseType) {
	bool found(false);
	SCRIPT_BEGIN(loadService(client))
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client,client,name.c_str())
			SCRIPT_WRITE_DATA(reader,0)
			found=true;
			SCRIPT_FUNCTION_CALL
			if(SCRIPT_CAN_READ) {
				DataWriter& writer = client.writer().writeResponse(responseType);
				SCRIPT_READ_DATA(writer);
			}
		SCRIPT_FUNCTION_END
		if(SCRIPT_LAST_ERROR)
			ex.set(Exception::SOFTWARE,SCRIPT_LAST_ERROR);
	SCRIPT_END
	return found;
}

bool MonaServer::onFileAccess(Exception& ex, Client& client, Client::FileAccessType type, Path& filePath,DataReader& parameters,DataWriter& properties) { 

	bool result = true;
	string name;
	if (client.path!=filePath.toString()) // "" if it is current application
		name.assign(filePath.name());
	SCRIPT_BEGIN(loadService(client))
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client,client,type==Client::FileAccessType::READ ? "onRead" : "onWrite")
			SCRIPT_WRITE_STRING(name.c_str())
			SCRIPT_WRITE_DATA(parameters,0)
			SCRIPT_FUNCTION_CALL
			if(SCRIPT_CAN_READ) {
				if (SCRIPT_NEXT_TYPE == LUA_TNIL) {
					result=false;
					SCRIPT_READ_NIL
				} else {
					// Redirect to the file (get name to prevent path insertion)
					FileSystem::GetName(SCRIPT_READ_STRING(name), name);
				}
				if(SCRIPT_NEXT_TYPE==LUA_TTABLE) {
					lua_pushnil(_pState);  // first key 
					while (lua_next(_pState, -2) != 0) {
						// uses 'key' (at index -2) and 'value' (at index -1) 
						if (lua_isstring(_pState, -2)) {
							properties.writePropertyName(lua_tostring(_pState,-2));
							Script::ReadData(_pState,properties,1);
							properties.endWrite(); // TODO remove the caller can continue to write the properties
						} else
							SCRIPT_WARN("key=value ignored because key is not a string value")
						lua_pop(_pState,1);
					}
					SCRIPT_READ_NEXT
				}
			}
		SCRIPT_FUNCTION_END
		if (SCRIPT_LAST_ERROR) {
			ex.set(Exception::SOFTWARE, SCRIPT_LAST_ERROR);
			return false;
		}
	SCRIPT_END

	filePath.set(WWWPath, client.path);
	if (!name.empty())
		filePath.append('/', name);
	return result;
}


//// PUBLICATION_HANDLER /////
bool MonaServer::onPublish(Client& client,const Publication& publication,string& error) {
	Script::AddObject<LUAPublication>(_pState, publication);
	bool result(true);
	if (client != this->id) {
		SCRIPT_BEGIN(loadService(client))
			SCRIPT_MEMBER_FUNCTION_BEGIN(Client,client,"onPublish")
				lua_pushvalue(_pState, 1); // publication! (see Script::AddObject above)
				SCRIPT_FUNCTION_CALL
				if(SCRIPT_CAN_READ)
					result = SCRIPT_READ_BOOL(true);
			SCRIPT_FUNCTION_END
			if(SCRIPT_LAST_ERROR) {
				error = SCRIPT_LAST_ERROR;
				result = false;
			}
		SCRIPT_END
	}
	if (result)
		LUAInvoker::AddPublication(_pState, publication);
	else if (publication.listeners.count() == 0)
		Script::ClearObject<LUAPublication>(_pState, publication);
	lua_pop(_pState, 1); // remove Script::AddObject<Publication,... (see above)
	return result;
}

void MonaServer::onUnpublish(Client& client,const Publication& publication) {
	if (client != this->id) {	
		SCRIPT_BEGIN(loadService(client))
			SCRIPT_MEMBER_FUNCTION_BEGIN(Client,client,"onUnpublish")
				Script::AddObject<LUAPublication>(_pState, publication);
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	}
	LUAInvoker::RemovePublication(_pState, publication);
	if (publication.listeners.count() == 0)
		Script::ClearObject<LUAPublication>(_pState, publication);
}

bool MonaServer::onSubscribe(Client& client,const Listener& listener,string& error) { 
	Script::AddObject<LUAListener>(_pState, listener);
	Script::AddObject<LUAPublication>(_pState, listener.publication);
	
	bool result = true;
	bool done(false);
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_WITH_OBJHANDLE_BEGIN(Client, client, "onSubscribe", LUAPublication::AddListener(_pState, 2, 1); done = true;)
			lua_pushvalue(_pState, 1); // listener! (see Script::AddObject above)
			SCRIPT_FUNCTION_CALL
			if (SCRIPT_CAN_READ)
				result = SCRIPT_READ_BOOL(true);
		SCRIPT_FUNCTION_END
		if (SCRIPT_LAST_ERROR) {
			error = SCRIPT_LAST_ERROR;
			result = false;
		}
	SCRIPT_END
	if (!result) {
		LUAPublication::RemoveListener(_pState, listener.publication);
		Script::ClearObject<LUAListener>(_pState, listener);
		if (!listener.publication.publisher() && listener.publication.listeners.count() == 0)
			Script::ClearObject<LUAPublication>(_pState, listener.publication);
	} else if (!done && Script::FromObject<Client>(_pState, client)) {
		LUAPublication::AddListener(_pState, 2, 1);
		lua_pop(_pState, 1);
	}
	lua_pop(_pState, 2); // remove Script::AddObject<Listener,... (see above) and Publication
	return result;
}

void MonaServer::onUnsubscribe(Client& client,const Listener& listener) {
	bool done(false);
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client, client,"onUnsubscribe")
			Script::AddObject<LUAListener>(_pState, listener);
			LUAPublication::RemoveListener(_pState, listener.publication);
			done = true;
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END

	if (!listener.publication.publisher() && listener.publication.listeners.count() == 0)
		Script::ClearObject<LUAPublication>(_pState, listener.publication);
	else if (!done && Script::FromObject<Listener>(_pState, listener)) {
		LUAPublication::RemoveListener(_pState, listener.publication);
		lua_pop(_pState, 1);
	}
	Script::ClearObject<LUAListener>(_pState, listener);
}

void MonaServer::onAudioPacket(Client& client,const Publication& publication,UInt32 time,PacketReader& packet) {
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onAudio")
			SCRIPT_WRITE_NUMBER(time)
			SCRIPT_WRITE_BINARY(packet.current(),packet.available())
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onVideoPacket(Client& client,const Publication& publication,UInt32 time,PacketReader& packet) {
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onVideo")
			SCRIPT_WRITE_NUMBER(time)
			SCRIPT_WRITE_BINARY(packet.current(),packet.available())
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onDataPacket(Client& client,const Publication& publication,DataReader& packet) {
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onData")
			SCRIPT_WRITE_DATA(packet,0)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onFlushPackets(Client& client,const Publication& publication) {
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onFlush")
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onJoinGroup(Client& client,Group& group) {
	Script::AddObject<LUAGroup>(_pState, group);
	LUAInvoker::AddGroup(_pState);
	bool done(false);
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_WITH_OBJHANDLE_BEGIN(Client, client, "onJoinGroup", LUAGroup::AddClient(_pState, 1); done = true;)
			lua_pushvalue(_pState, 1); // group! (see Script::AddObject above)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
	if (!done && Script::FromObject<Client>(_pState, client)) {
		LUAGroup::AddClient(_pState,1);
		lua_pop(_pState, 1);
	}
	lua_pop(_pState, 1); // remove Script::AddObject<Group,... (see above)	
}

void MonaServer::onUnjoinGroup(Client& client,Group& group) {
	bool done(false);
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client, client,"onUnjoinGroup")
			Script::AddObject<LUAGroup>(_pState, group);
			LUAGroup::RemoveClient(_pState,client);
			done = true;
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
	if (group.count() == 0) {
		if (Script::FromObject<Group>(_pState, group)) {
			LUAInvoker::RemoveGroup(_pState);
			Script::ClearObject<LUAGroup>(_pState,group);
			lua_pop(_pState, 1);
		}
	} else if (!done && Script::FromObject<Group>(_pState,group)) {
		LUAGroup::RemoveClient(_pState, client);
		lua_pop(_pState, 1);
	}
}

