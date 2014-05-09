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

MonaServer::MonaServer(TerminateSignal& terminateSignal, const Parameters& configs) :
	Server(configs.getNumber<UInt32>("socketBufferSize"), configs.getNumber<UInt16>("threads")), servers(configs,sockets), _firstData(true),_data(this->poolBuffers),_terminateSignal(terminateSignal),
	setLUAProperty([this](const string& key, const string& value) {
		if (String::ICompare(value, "false") == 0 || String::ICompare(value, "nil") == 0)
			lua_pushboolean(_pState, 0);
		else
			lua_pushlstring(_pState, value.c_str(), value.size());
		lua_setfield(_pState, -2, key.c_str());
	}) {


	Parameters::ForEach forEach([this](const string& key, const string& value) {
		setString(key, value);
	});
	configs.iterate(forEach);

	string pathApp;
	configs.getString("application.dir", pathApp);
	(string&)WWWPath = pathApp + "www";
	(string&)DataPath = pathApp + "data";


	onServerConnection = [this](ServerConnection& server) {
		Script::AddObject<ServerConnection, LUAServer>(_pState, server);
		LUABroadcaster::AddServer(_pState,servers, server.address.toString());
		LUABroadcaster::AddServer(_pState, server.isTarget ? servers.targets : servers.initiators, server.address.toString());
		bool error(false);
		SCRIPT_BEGIN(_pService->open())
			SCRIPT_FUNCTION_BEGIN("onServerConnection")
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
				Expirable<Service> expirableService;
				Service* pService = _pService->get(path, expirableService);
				if (pService)
					pService->open();
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

	onServerDisconnection = [this](Exception& ex,const ServerConnection& server) {
		SCRIPT_BEGIN(_pService->open())
			SCRIPT_FUNCTION_BEGIN("onServerDisconnection")
				SCRIPT_ADD_OBJECT(ServerConnection,LUAServer,server)
				if (ex)
					SCRIPT_WRITE_STRING(ex.error().c_str())
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
		LUABroadcaster::RemoveServer(_pState, servers, server.address.toString());
		LUABroadcaster::RemoveServer(_pState, server.isTarget ? servers.targets : servers.initiators, server.address.toString());
		Script::RemoveObject<ServerConnection, LUAServer>(_pState, server);
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
			SCRIPT_FUNCTION_BEGIN("onServerConnection")
				SCRIPT_ADD_OBJECT(ServerConnection,LUAServer,*pServer)
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
			SCRIPT_FUNCTION_BEGIN("onServerDisconnection")
				SCRIPT_ADD_OBJECT(ServerConnection,LUAServer,*pServer)
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		}
	SCRIPT_END
}

void MonaServer::onStart() {
	
	_pState = Script::CreateState();

	// init root server application
	_pService.reset(new Service(_pState, "", *this));

	// init few global variable
	SCRIPT_BEGIN(_pState)
		// Configs + Environement
		Script::Collection(_pState,LUA_GLOBALSINDEX, "m.c", count());
		iterate(setLUAProperty);
		Script::Collection(_pState,LUA_GLOBALSINDEX, "m.e", Util::Environment().count());
		Util::Environment().iterate(setLUAProperty);
		lua_pop(_pState, 2);

		SCRIPT_ADD_OBJECT(Invoker, LUAInvoker, *this)
		lua_setglobal(_pState,"mona");
		SCRIPT_ADD_OBJECT(Broadcaster, LUABroadcaster, servers)
		lua_setglobal(_pState, "m.s");
		SCRIPT_ADD_OBJECT(Broadcaster, LUABroadcaster, servers.initiators)
		lua_setglobal(_pState, "m.s.i");
		SCRIPT_ADD_OBJECT(Broadcaster, LUABroadcaster, servers.targets)
		lua_setglobal(_pState, "m.s.t");
		lua_getmetatable(_pState, LUA_GLOBALSINDEX);
		SCRIPT_NEW_OBJECT(LUADataTable, LUADataTable, new LUADataTable(_data, _pService->path));
		lua_setfield(_pState, -2, "|data");
		lua_pop(_pState, 1);
	SCRIPT_END

	// load database
	Exception ex;
	_firstData = true;
	if (_data.load(ex, DataPath, *this, true)) {
		if (ex)
			ERROR("Error on database loading, ", ex)
		NOTE("Database loaded")
	}
	
	// start servers
	servers.start();

	// start the application (if exists)
	_pService->watchFile();
}


void MonaServer::onDataLoading(const string& path, const char* value, UInt32 size) {
	if (_firstData) {
		INFO("Database loading...")
		_firstData = false;
	}
	if (!_pState) {
		ERROR("Loading database useless, no recipient")
		return;
	}
	// get table and set key
	lua_getmetatable(_pState, LUA_GLOBALSINDEX);
	string key("|data");
	if (!path.empty()) {
		lua_getfield(_pState, -1, "|data");
		lua_replace(_pState, -2);
		if (lua_isnil(_pState, -1)) {
			lua_pop(_pState, 1);
			ERROR("Loading database impossible because no recipient")
				return;
		}
	}
	vector<string> values;
	String::Split(path, "/", values, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM);
	if (!values.empty()) {
		key.assign(values.back());
		values.pop_back();
	}
	for (const string& subpath : values) {
		if (!lua_istable(_pState, -1)) {
			lua_pop(_pState, 1);
			WARN("Loading database entry ", path, " ignored because parent is not a table")
				return;
		}
		lua_getfield(_pState, -1, subpath.c_str());
		if (lua_isnil(_pState, -1)) {
			lua_pop(_pState, 1);
			lua_newtable(_pState);
			lua_setfield(_pState, -2, subpath.c_str());
			lua_getfield(_pState, -1, subpath.c_str());
		}
		lua_replace(_pState, -2);
	}

	// set value
	string val(value, size);
	setLUAProperty(key, val);

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
	if (_data.writing()) {
		INFO("Database flushing...")
		_data.flush();
	}
	servers.stop();
	_terminateSignal.set();
}


void MonaServer::manage() {
	Server::manage();
	servers.manage();
	if (!_pService)
		return;
	_pService->watchFile();
	SCRIPT_BEGIN(_pService->open())
		SCRIPT_FUNCTION_BEGIN("onManage")
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
	if (!server.getNumber(String::Format(buffer, protocol, ".port"), port))
		server.getNumber(String::Format(buffer, protocol, ".defaultPort"), port);
	if (port == 0) {
		SCRIPT_BEGIN(_pState)
			SCRIPT_ERROR("Impossible to determine ", protocol, " port of ", server.address.toString(), " server");
		SCRIPT_END
		return;
	}

	if (!server.getString(String::Format(buffer, protocol, ".publicHost"), buffer) && !server.getString("publicHost", buffer))
		buffer = server.address.host().toString();

	addLUAAddress(addresses,buffer, port);
}

lua_State* MonaServer::openService(const Client& client) {
	Expirable<Service>* pExpirableService = client.getUserData<Expirable<Service>>();
	if (!pExpirableService)
		return NULL;
	Service* pService = pExpirableService->unsafeThis();
	if (pService)
		return pService->open();
	return NULL;
}


void MonaServer::onRendezVousUnknown(const string& protocol,const UInt8* id,set<SocketAddress>& addresses) {
	SCRIPT_BEGIN(_pService->open())
		SCRIPT_FUNCTION_BEGIN("onRendezVousUnknown")
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
	Expirable<Service> expirableService;
	Service* pService = _pService->get(path, expirableService);
	if (!pService)
		return;
	SCRIPT_BEGIN(pService->open())
		SCRIPT_FUNCTION_BEGIN("onHandshake")
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
	Expirable<Service> expirableService;
	Service* pService = _pService->get(client.path, expirableService);
	if (!pService) {
		ex.set(Exception::APPLICATION, "Applicaton ", client.path, " doesn't exist");
		return;
	}

	// control script size to debug!
	if (lua_gettop(_pState) > 0)
		CRITIC("LUA stack corrupted");

	Script::AddObject<Client, LUAClient>(_pState,client);

	const char* error(NULL);
	SCRIPT_BEGIN(pService->open())
		SCRIPT_FUNCTION_BEGIN("onConnection")
			lua_pushvalue(_pState, 1); // client! (see Script::AddObject above)
			SCRIPT_WRITE_DATA(parameters,0)
			SCRIPT_FUNCTION_CALL_WITHOUT_LOG
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
			error = strlen(SCRIPT_LAST_ERROR)>0 ? SCRIPT_LAST_ERROR : "client rejected";	
	SCRIPT_END

	if (!error && pService->lastError.empty()) {
		LUAInvoker::AddClient(_pState,*this,client);
		lua_pop(_pState, 1); // remove Script::AddObject<Client .. (see above)
		// connection accepted
		expirableService.shareThis(client.setUserData<Expirable<Service>>(*new Expirable<Service>()));
		return;
	}
	// connection failed
	Script::RemoveObject<Client, LUAClient>(_pState,1);
	ex.set(Exception::SOFTWARE, error ? error : pService->lastError);
	lua_pop(_pState, 1); // remove Script::AddObject<Client .. (see above)
}

void MonaServer::onDisconnection(const Client& client) {
	Expirable<Service>* pExpirableService = client.getUserData<Expirable<Service>>();
	if (!pExpirableService)
		return;
	Service* pService = pExpirableService->unsafeThis();
	if (pService) {
		SCRIPT_BEGIN(pService->open())
			SCRIPT_FUNCTION_BEGIN("onDisconnection")
				SCRIPT_ADD_OBJECT(Client, LUAClient,client)
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	}
	LUAInvoker::RemoveClient(_pState, *this, client);
	Script::RemoveObject<Client, LUAClient>(_pState, client);
	delete pExpirableService;
}

bool MonaServer::onMessage(Exception& ex, Client& client,const string& name,DataReader& reader, UInt8 responseType) {
	bool found(false);
	SCRIPT_BEGIN(openService(client))
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

bool MonaServer::onRead(Exception& ex, Client& client,FilePath& filePath,DataReader& parameters,DataWriter& properties) { 
	bool result = true;
	filePath.setDirectory(WWWPath);
	SCRIPT_BEGIN(openService(client))
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client,client,"onRead")
			SCRIPT_WRITE_STRING((client.path==filePath.path())? "" : filePath.name().c_str()) // "" if it is current application
			SCRIPT_WRITE_DATA(parameters,0)
			SCRIPT_FUNCTION_CALL
			if(SCRIPT_CAN_READ) {
				if (SCRIPT_NEXT_TYPE == LUA_TNIL) {
					result=false;
					SCRIPT_READ_NIL
				} else {
					// Redirect to the file (get name to prevent path insertion)
					FileSystem::GetName(SCRIPT_READ_STRING(filePath.path()), buffer);
					if (!buffer.empty()) // to avoid for root app to give "/" instead of ""
						filePath.setPath(client.path,"/",buffer);
				}
				if(SCRIPT_NEXT_TYPE==LUA_TTABLE) {
					lua_pushnil(_pState);  // first key 
					while (lua_next(_pState, -2) != 0) {
						// uses 'key' (at index -2) and 'value' (at index -1) 
						if (lua_isstring(_pState, -2)) {
							properties.writePropertyName(lua_tostring(_pState,-2));
							Script::ReadData(_pState,properties,1);
							properties.endWrite();
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
	return result;
}


//// PUBLICATION_HANDLER /////
bool MonaServer::onPublish(Client& client,const Publication& publication,string& error) {
	Script::AddObject<Publication, LUAPublication<>>(_pState, publication);
	if (client == this->id) {
		LUAInvoker::AddPublication(_pState, *this, publication);
		lua_pop(_pState, 1); // remove Script::AddObject<Publication,... (see above)
		return true;
	}
	bool result=true;
	SCRIPT_BEGIN(openService(client))
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
	if (result)
		LUAInvoker::AddPublication(_pState, *this, publication);
	else if (publication.listeners.count() == 0)
		Script::RemoveObject<Publication, LUAPublication<>>(_pState, 1);
	lua_pop(_pState, 1); // remove Script::AddObject<Publication,... (see above)
	return result;
}

void MonaServer::onUnpublish(Client& client,const Publication& publication) {
	if(client != this->id) {
		SCRIPT_BEGIN(openService(client))
			SCRIPT_MEMBER_FUNCTION_BEGIN(Client,client,"onUnpublish")
				SCRIPT_ADD_OBJECT(Publication, LUAPublication<>, publication)
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	}
	LUAInvoker::RemovePublication(_pState, *this, publication);
	if (publication.listeners.count() == 0)
		Script::RemoveObject<Publication, LUAPublication<>>(_pState, publication);
}

bool MonaServer::onSubscribe(Client& client,const Listener& listener,string& error) { 
	Script::AddObject<Listener, LUAListener>(_pState, listener);
	bool result = true;
	bool done(false);
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_WITH_OBJHANDLE_BEGIN(Client, client, "onSubscribe", LUAPublication<>::AddListener(_pState, listener, 1); done = true;)
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
		LUAPublication<>::RemoveListener(_pState, listener);
		Script::RemoveObject<Listener, LUAListener>(_pState, listener);
	} else if (!done && Script::FromObject<Client>(_pState, client)) {
		LUAPublication<>::AddListener(_pState, listener, 1);
		lua_pop(_pState, 1);
	}
	lua_pop(_pState, 1); // remove Script::AddObject<Listener,... (see above)
	return result;
}

void MonaServer::onUnsubscribe(Client& client,const Listener& listener) {
	bool done(false);
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client, client,"onUnsubscribe")
			SCRIPT_ADD_OBJECT(Listener, LUAListener, listener)
			LUAPublication<>::RemoveListener(_pState, listener);
			done = true;
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END

	if (!listener.publication.publisher() && listener.publication.listeners.count() == 0)
		Script::RemoveObject<Publication, LUAPublication<>>(_pState, listener.publication);
	else if (!done && Script::FromObject<Listener>(_pState, listener)) {
		LUAPublication<>::RemoveListener(_pState, listener);
		lua_pop(_pState, 1);
	}
}

void MonaServer::onAudioPacket(Client& client,const Publication& publication,UInt32 time,PacketReader& packet) {
	if(client == this->id)
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onAudio")
			SCRIPT_WRITE_NUMBER(time)
			SCRIPT_WRITE_BINARY(packet.current(),packet.available())
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onVideoPacket(Client& client,const Publication& publication,UInt32 time,PacketReader& packet) {
	if(client == this->id)
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onVideo")
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
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onData")
			SCRIPT_WRITE_DATA(packet,0)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onFlushPackets(Client& client,const Publication& publication) {
	if(client == this->id)
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onFlush")
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onJoinGroup(Client& client,Group& group) {
	Script::AddObject<Group, LUAGroup>(_pState, group);
	LUAInvoker::AddGroup(_pState, *this, group);
	bool done(false);
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_WITH_OBJHANDLE_BEGIN(Client, client, "onJoinGroup", LUAGroup::AddClient(_pState, group, client, 1); done = true;)
			lua_pushvalue(_pState, 1); // group! (see Script::AddObject above)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
	if (!done && Script::FromObject<Client>(_pState, client)) {
		LUAGroup::AddClient(_pState, group, client,1);
		lua_pop(_pState, 1);
	}
	lua_pop(_pState, 1); // remove Script::AddObject<Group,... (see above)	
}

void MonaServer::onUnjoinGroup(Client& client,Group& group) {
	bool done(false);
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client, client,"onUnjoinGroup")
			SCRIPT_ADD_OBJECT(Group, LUAGroup, group)
			LUAGroup::RemoveClient(_pState, group,client);
			done = true;
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
	if (group.count() == 0) {
		LUAInvoker::RemoveGroup(_pState, *this, group);
		Script::RemoveObject<Group, LUAGroup>(_pState, group);
	} else if (!done && Script::FromObject<Group>(_pState,group)) {
		LUAGroup::RemoveClient(_pState, group, client);
		lua_pop(_pState, 1);
	}
}

