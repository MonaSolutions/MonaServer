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
#include "LUAPersistentTable.h"
#include "LUASocketAddress.h"
#include "LUAIPAddress.h"
#include "ScriptWriter.h"
#include "ScriptReader.h"
#include "Mona/AMFReader.h"


using namespace std;
using namespace Mona;

MonaServer::MonaServer(const Parameters& configs, TerminateSignal& terminateSignal) : _pState(NULL),
	Server(configs.getNumber<UInt32>("socketBufferSize"), configs.getNumber<UInt16>("threads")), servers(sockets), _data(this->poolBuffers),_terminateSignal(terminateSignal),
	setLUAProperty([this](const string& key, const string& value) { Script::PushValue(_pState, value); lua_setfield(_pState, -2, key.c_str());} ) {
	
	string pathApp("./");
	configs.getString("application.dir", pathApp);
	_wwwPath.assign(pathApp).append("www");
	_dataPath.assign(pathApp).append("data");

	onPublicationData = [this](const Publication& publication,DataReader& data) {
		SCRIPT_BEGIN(_pState)
			SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onData")
				ScriptWriter writer(_pState);
				data.read(writer);
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	};

	onPublicationAudio = [this](const Publication& publication,UInt32 time,PacketReader& packet) {
		SCRIPT_BEGIN(_pState)
			SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onAudio")
				SCRIPT_WRITE_NUMBER(time)
				SCRIPT_WRITE_BINARY(packet.current(),packet.available())
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	};

	onPublicationVideo = [this](const Publication& publication,UInt32 time,PacketReader& packet) {
		SCRIPT_BEGIN(_pState)
			SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onVideo")
				SCRIPT_WRITE_NUMBER(time)
				SCRIPT_WRITE_BINARY(packet.current(),packet.available())
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	};

	onPublicationFlush = [this](const Publication& publication) {
		SCRIPT_BEGIN(_pState)
			SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onFlush")
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	};


	onPublicationProperties = [this](const Publication& publication, const Parameters& properties) {
		SCRIPT_BEGIN(_pState)
			SCRIPT_MEMBER_FUNCTION_BEGIN(Publication, publication, "onProperties")
				if (properties.empty()) {
					Script::DeleteCollection(_pState, -1, "properties");
					lua_pushnil(_pState);
				} else {
					Script::Collection(_pState, -1, "properties");		
					Parameters::ForEach forEach([this](const std::string& key, const std::string& value) {
						Script::PushKeyValue(_pState, key, value);
					});
					Script::ClearCollection(_pState);
					Script::FillCollection(_pState, properties.iterate(forEach));	
				}
				lua_pushvalue(_pState,-1); // properties collection
				lua_setfield(_pState,-3,"properties"); // fix on publication object
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	};


	onServerConnection = [this](ServerConnection& server) {
		Script::AddObject<LUAServer>(_pState, server);
		LUABroadcaster::AddServer(_pState,servers, server.address.toString());
		LUABroadcaster::AddServer(_pState, server.isTarget ? servers.targets : servers.initiators, server.address.toString());

		// Server connection must open the service if not always loaded!
		Exception ex;
		if (!_pService->open(ex)) {
			server.reject(ex.error());
			return;
		}
		
		bool error(false);
		SCRIPT_BEGIN(_pState)
			SCRIPT_FUNCTION_BEGIN("onServerConnection",_pService->reference())
				lua_pushvalue(_pState, 1); // server! (see Script::AddObject above)
				SCRIPT_FUNCTION_CALL
				if (SCRIPT_FUNCTION_ERROR) {
					error = true;
					server.reject(SCRIPT_FUNCTION_ERROR);
				}
			SCRIPT_FUNCTION_END
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
				ScriptWriter writer(_pState);
				AMFReader(packet).read(writer);
				SCRIPT_FUNCTION_CALL;
			SCRIPT_FUNCTION_END;
		SCRIPT_END;
	};

	onServerDisconnection = [this](const Exception& ex,const ServerConnection& server) {
		SCRIPT_BEGIN(_pState)
			SCRIPT_FUNCTION_BEGIN("onServerDisconnection",_pService->reference())
				Script::AddObject<LUAServer>(_pState, server);
				if (ex)
					SCRIPT_WRITE_STRING(ex.error())
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	
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


bool MonaServer::start(const Parameters& configs) {
	if(Server::running()) {
		ERROR("Server is already running, call stop method before");
		return false;
	}

	Exception ex;
	EXCEPTION_TO_LOG(FileSystem::CreateDirectory(ex, _wwwPath),"Application directory creation");
	return Server::start(configs);
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
				if (SCRIPT_FUNCTION_ERROR)
					pServer->reject(SCRIPT_FUNCTION_ERROR);
			SCRIPT_FUNCTION_END
		}
	SCRIPT_END

}
void MonaServer::stopService(Service& service) {
	_servicesRunning.erase(&service);

	// Call all the onServerDisconnection event for every services where service.path is running
	SCRIPT_BEGIN(_pState)
		for(ServerConnection* pServer : servers) {
			SCRIPT_FUNCTION_BEGIN("onServerDisconnection", service.reference())
				Script::AddObject<LUAServer>(_pState, *pServer);
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		}
	SCRIPT_END
}

void MonaServer::onStart() {
	
	_pState = Script::CreateState();


	// init root server application
	_pService.reset(new Service(_pState, _wwwPath, *this));

	// Init few global variable
	Script::AddObject<LUAInvoker,Invoker>(_pState,*this);
	lua_getmetatable(_pState, -1);

	Script::AddObject<LUABroadcaster,Broadcaster>(_pState, servers);
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

	Script::NewObject<LUAPersistentTable>(_pState, *new LUAPersistentTable(_data, _pService->path));
	lua_setfield(_pState, LUA_REGISTRYINDEX, "|data");
	
	// load database
	Exception ex;
	bool firstData(true);
	PersistentData::ForEach forEach([this,&firstData](const string& path, const UInt8* value, UInt32 size) {
		if (firstData) {
			INFO("Database loading...")
			firstData = false;
		}
		if (!_pState) {
			ERROR("Databas load useless, no recipient")
			return;
		}
		// get table and set key
		lua_pushvalue(_pState,LUA_REGISTRYINDEX);
		if (!path.empty()) {
			lua_getfield(_pState, LUA_REGISTRYINDEX, "|data");
			lua_replace(_pState, -2);
			if (lua_isnil(_pState, -1)) {
				lua_pop(_pState, 1);
				ERROR("Databas load impossible because no recipient")
				return;
			}
		}

		string key;
		string newPath(path);
		if (FileSystem::GetName(path,key).empty())
			key = "|data";
		else
			FileSystem::GetParent(newPath);
		

		String::ForEach forEach([this,&newPath](UInt32 index, const char* field) {
			if (!lua_istable(_pState, -1)) {
				lua_pop(_pState, 1);
				WARN("Databas load entry ", newPath, " ignored because parent is not a table")
					return false;
			}
			lua_getfield(_pState, -1, field);
			if (lua_isnil(_pState, -1)) {
				lua_pop(_pState, 1);
				lua_newtable(_pState);
				lua_setfield(_pState, -2, field);
				lua_getfield(_pState, -1, field);
			}
			lua_replace(_pState, -2);
			return true;
		});
		if (String::Split(newPath, "/", forEach, String::SPLIT_IGNORE_EMPTY | String::SPLIT_TRIM) == string::npos)
			return;

		// set value
		Script::PushValue(_pState,value,size);
		lua_setfield(_pState, -2, key.c_str());
		lua_pop(_pState, 1); // remove table
	});


	_data.load(ex, _dataPath, forEach, true);
	if (ex)
		WARN("Database load, ",ex.error())
	else if (!firstData)
		NOTE("Database loaded")

	// start servers
	servers.start(*this);

	// start main application (if exists)
	ex.set(Exception::NIL);
	_pService->open(ex);
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
	// update the main service on manage
	if (!_pService->open(ex))
		return;
	SCRIPT_BEGIN(_pState)
		SCRIPT_FUNCTION_BEGIN("onManage",_pService->reference())
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::readAddressRedirection(const string& protocol, int& index, set<SocketAddress>& addresses) {

	SCRIPT_BEGIN(_pState)

		Exception ex;
		SocketAddress address;

		if (lua_type(_pState, index)==LUA_TSTRING) { // lua_type because can be encapsulated in a lua_next
			if (String::ICompare(lua_tostring(_pState, index), "myself") == 0) {
				addresses.emplace(); // wilcard, will be changed to the server public address by the caller
				return;
			}
		} else if (lua_istable(_pState, index)) {
			bool isConst;
			Broadcaster* pBroadcaster = Script::ToObject<Broadcaster>(_pState, isConst,index);
			if (pBroadcaster) {
				for (ServerConnection* pServer : *pBroadcaster) {
					if (pServer->protocolAddress(ex, protocol, address)) {
						addresses.emplace(address);
						if (ex)
							SCRIPT_WARN("Redirection address, ",ex.error())
					} else
						SCRIPT_ERROR("Redirection address, ",ex.error())
				}
				return;
			} 
			ServerConnection* pServer = Script::ToObject<ServerConnection>(_pState, isConst, index);
			if (pServer) {
				if (pServer->protocolAddress(ex, protocol, address)) {
					addresses.emplace(address);
					if (ex)
						SCRIPT_WARN("Redirection address, ",ex.error())
				} else
					SCRIPT_ERROR("Redirection address, ",ex.error())
				return;
			}
		}

	
		if (LUASocketAddress::Read(ex, _pState, index, address)) {
			addresses.emplace(address);
			if (ex)
				SCRIPT_WARN(ex.error());
			return;
		}

		if (lua_istable(_pState, index)) {
			// is an array of addresses?
			lua_pushnil(_pState);  // first key
			while (lua_next(_pState, index<0 ? (index-1) : index) != 0) {
				// uses 'key' (at index -2) and 'value' (at index -1) 
				int index(-1);
				readAddressRedirection(protocol,index,addresses);
				lua_pop(_pState,1);
			}
			return;
		}

		if (ex)
			SCRIPT_ERROR(ex.error());

	SCRIPT_END

}

void MonaServer::onRendezVousUnknown(const string& protocol,const UInt8* id,set<SocketAddress>& addresses) {
	Exception ex;
	// don't open the main service if not loaded by onManage (useless)
	SCRIPT_BEGIN(_pState)
		SCRIPT_FUNCTION_BEGIN("onRendezVousUnknown",_pService->reference())
			SCRIPT_WRITE_STRING(protocol.c_str())
			SCRIPT_WRITE_BINARY(id,ID_SIZE)
			SCRIPT_FUNCTION_CALL
			while(SCRIPT_READ_AVAILABLE)
				readAddressRedirection(protocol,SCRIPT_READ_NEXT(1),addresses);
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

void MonaServer::onHandshake(const string& protocol,const SocketAddress& address,const string& path,const Parameters& properties,UInt32 attempts,set<SocketAddress>& addresses) {
	Exception ex;
	// open the relating service
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
			while(SCRIPT_READ_AVAILABLE)
				readAddressRedirection(protocol,SCRIPT_READ_NEXT(1),addresses);
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

//// CLIENT_HANDLER /////
void MonaServer::onConnection(Exception& ex, Client& client,DataReader& parameters,DataWriter& response) {
	// open/create the service on connection client
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
			ScriptWriter writer(_pState);
			parameters.read(writer);
			SCRIPT_FUNCTION_CALL
			if (SCRIPT_FUNCTION_ERROR)
					ex.set(Exception::SOFTWARE, strlen(SCRIPT_FUNCTION_ERROR)>0 ? SCRIPT_FUNCTION_ERROR : "client rejected");
			else if(SCRIPT_READ_AVAILABLE) {
				if(SCRIPT_NEXT_TYPE==LUA_TTABLE) {

					lua_pushnil(_pState);  // first key 
					while (Script::Next(_pState, -2) != 0) {
						// uses 'key' (at index -2) and 'value' (at index -1) 
						if (lua_type(_pState, -2)==LUA_TSTRING) { // lua_type because can be encapsulated in a lua_next
							response.writePropertyName(lua_tostring(_pState,-2));
							if (!ScriptReader(_pState, 1).read(response))
								response.writeNull();
						} else
							SCRIPT_WARN("onConnection property ignored because key is not a string value")
						lua_pop(_pState,1);
					}

					SCRIPT_READ_NEXT(1);
				} else
					SCRIPT_ERROR("onConnection properties returned must be a table {prop1=value1,prop2=value2}")
			}
		SCRIPT_FUNCTION_END
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

	lua_rawgeti(_pState, LUA_REGISTRYINDEX, *client.setCustomData<int>(new int(service.reference())));

	Script::Collection(_pState, -1, "clients");
	lua_getfield(_pState, -3,"id");
	lua_pushvalue(_pState, -4);
	Script::FillCollection(_pState, 1);
	lua_pop(_pState, 2);

	return _pState;
}

lua_State* MonaServer::closeService(const Client& client,int& reference) {
	// -1 must be client table
	if (!client.hasCustomData())
		return NULL;
	reference = *client.getCustomData<int>();
	delete client.getCustomData<int>();
	client.setCustomData<int>(NULL);
	if (!_pState || reference == LUA_REFNIL)
		return NULL;
	lua_rawgeti(_pState, LUA_REGISTRYINDEX, reference);
	Script::Collection(_pState, -1, "clients");
	lua_getmetatable(_pState, -3);
	lua_getfield(_pState, -1, "|id"); // to be sure that id is not overrided
	lua_replace(_pState, -2);
	lua_pushnil(_pState);
	Script::FillCollection(_pState, 1);

	lua_pop(_pState, 2);
	return _pState;
}

void MonaServer::onDisconnection(const Client& client) {
	Script::AddObject<LUAClient>(_pState, client);
	LUAInvoker::RemoveClient(_pState);
	
	int reference;
	SCRIPT_BEGIN(closeService(client,reference))
		SCRIPT_FUNCTION_BEGIN("onDisconnection",reference)
			lua_pushvalue(_pState, 1); // client! (see Script::AddObject above)
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END

	lua_pop(_pState, 1);  // remove Script::AddObject<Client .. (see above)

	Script::ClearObject<LUAClient>(_pState,client);
}

void MonaServer::onAddressChanged(Client& client,const SocketAddress& oldAddress) {
	SCRIPT_BEGIN(loadService(client))
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client, client, "onAddressChanged")
			SCRIPT_WRITE_STRING(oldAddress.toString().c_str())
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END
}

bool MonaServer::onMessage(Exception& ex, Client& client,const string& name,DataReader& reader, UInt8 responseType) {
	bool found(false);
	SCRIPT_BEGIN(loadService(client))
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client,client, name.c_str())
			ScriptWriter writer(_pState);
			reader.read(writer);
			found=true;
			SCRIPT_FUNCTION_CALL
			if(SCRIPT_FUNCTION_ERROR)
				ex.set(Exception::SOFTWARE,SCRIPT_FUNCTION_ERROR);
			else if (SCRIPT_READ_AVAILABLE)
				SCRIPT_READ_NEXT(ScriptReader(_pState, SCRIPT_READ_AVAILABLE).read(client.writer().writeResponse(responseType)));
		SCRIPT_FUNCTION_END
		
	SCRIPT_END
	return found;
}

bool MonaServer::onFileAccess(Exception& ex, Client& client, Client::FileAccessType type, DataReader& parameters, File& file, DataWriter& properties) { 

	if (file.isFolder()) {
		// filePath must be a file, not a folder, otherwise it's a security issue
		ex.set(Exception::PERMISSION, "Impossible to access to ",file.path()," ressource through the ",client.path," application, this resource is managed by ",file.parent()," application");
		return false;
	}

	bool result(true);
	file.setParent(_wwwPath,  client.path);

	SCRIPT_BEGIN(loadService(client))
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client,client,type==Client::FileAccessType::READ ? "onRead" : "onWrite")
			SCRIPT_WRITE_STRING(file.name().c_str())
			ScriptWriter writer(_pState);
			parameters.read(writer);
			SCRIPT_FUNCTION_CALL
			if (SCRIPT_FUNCTION_ERROR) {
				ex.set(Exception::SOFTWARE, SCRIPT_FUNCTION_ERROR);
				result = false;
			} else if(SCRIPT_READ_AVAILABLE) {
				if (SCRIPT_NEXT_TYPE == LUA_TNIL) {
					result=false;
					SCRIPT_READ_NIL
				} else {
					// Redirect to the file (get name to prevent path insertion)
					string name;
					if(!file.setName(FileSystem::GetName(SCRIPT_READ_STRING(file.name()), name)))
						file.setPath(_wwwPath,  client.path,"/"); // redirect to folder view
				}
				if(SCRIPT_NEXT_TYPE==LUA_TTABLE) {
					lua_pushnil(_pState);  // first key 
					while (Script::Next(_pState, -2) != 0) {
						// uses 'key' (at index -2) and 'value' (at index -1) 
						if (lua_type(_pState,-2)==LUA_TSTRING) {
							properties.writePropertyName(lua_tostring(_pState,-2));
							if (!ScriptReader(_pState, 1).read(properties))
								properties.writeNull();
						} else
							SCRIPT_WARN("key=value ignored because key is not a string value")
						lua_pop(_pState,1);
					}
					SCRIPT_READ_NEXT(1);
				} else if (SCRIPT_READ_AVAILABLE)
					SCRIPT_ERROR(type==Client::FileAccessType::READ ? "onRead" : "onWrite"," properties returned must be a table {prop1=value1,prop2=value2}")
			}
		SCRIPT_FUNCTION_END
	SCRIPT_END

	return result;
}


//// PUBLICATION_HANDLER /////
bool MonaServer::onPublish(Exception& ex, const Publication& publication, Client* pClient) {
	Script::AddObject<LUAPublication>(_pState, publication);
	bool result(true);
	if (pClient) {
		SCRIPT_BEGIN(loadService(*pClient))
			SCRIPT_MEMBER_FUNCTION_BEGIN(Client,*pClient,"onPublish")
				lua_pushvalue(_pState, 1); // publication! (see Script::AddObject above)
				SCRIPT_FUNCTION_CALL
				if(SCRIPT_FUNCTION_ERROR) {
					ex.set(Exception::APPLICATION, SCRIPT_FUNCTION_ERROR);
					result = false;
				} else if(SCRIPT_READ_AVAILABLE)
					result = SCRIPT_READ_BOOL(true);
			SCRIPT_FUNCTION_END
		SCRIPT_END
	}
	if (result) {
		LUAInvoker::AddPublication(_pState, publication);

		publication.OnAudio::subscribe(onPublicationAudio);
		publication.OnVideo::subscribe(onPublicationVideo);
		publication.OnData::subscribe(onPublicationData);
		publication.OnFlush::subscribe(onPublicationFlush);
		publication.OnProperties::subscribe(onPublicationProperties);

	} else if (publication.listeners.count() == 0)
		Script::ClearObject<LUAPublication>(_pState, publication);
	lua_pop(_pState, 1); // remove Script::AddObject<Publication,... (see above)
	return result;
}

void MonaServer::onUnpublish(const Publication& publication, Client* pClient) {
	if (pClient) {	
		SCRIPT_BEGIN(loadService(*pClient))
			SCRIPT_MEMBER_FUNCTION_BEGIN(Client,*pClient,"onUnpublish")
				Script::AddObject<LUAPublication>(_pState, publication);
				SCRIPT_FUNCTION_CALL
			SCRIPT_FUNCTION_END
		SCRIPT_END
	}
	LUAInvoker::RemovePublication(_pState, publication);

	publication.OnAudio::unsubscribe(onPublicationAudio);
	publication.OnVideo::unsubscribe(onPublicationVideo);
	publication.OnData::unsubscribe(onPublicationData);
	publication.OnFlush::unsubscribe(onPublicationFlush);
	publication.OnProperties::unsubscribe(onPublicationProperties);

	if (publication.listeners.count() == 0)
		Script::ClearObject<LUAPublication>(_pState, publication);
}

bool MonaServer::onSubscribe(Exception& ex, Client& client,const Listener& listener) { 
	Script::AddObject<LUAListener>(_pState, listener);
	Script::AddObject<LUAPublication>(_pState, listener.publication);
	
	bool result = true;
	bool done(false);
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client, client, "onSubscribe")
			LUAPublication::AddListener(_pState, 2, 1);
			done = true;
			lua_pushvalue(_pState, 1); // listener! (see Script::AddObject above)
			SCRIPT_FUNCTION_CALL
			if (SCRIPT_FUNCTION_ERROR) {
				ex.set(Exception::APPLICATION,SCRIPT_FUNCTION_ERROR);
				result = false;
			} else if (SCRIPT_READ_AVAILABLE)
				result = SCRIPT_READ_BOOL(true);
		SCRIPT_FUNCTION_END
	SCRIPT_END
	if (!result) {
		if (Script::FromObject<Client>(_pState, client)) {
			LUAPublication::RemoveListener(_pState, listener.publication);
			Script::ClearObject<LUAListener>(_pState, listener);
			if (!listener.publication.running() && listener.publication.listeners.count() == 0)
				Script::ClearObject<LUAPublication>(_pState, listener.publication);
			lua_pop(_pState, 1);
		}
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
			LUAPublication::RemoveListener(_pState, listener.publication);
			done = true;
			SCRIPT_FUNCTION_CALL
		SCRIPT_FUNCTION_END
	SCRIPT_END

	if (!listener.publication.running() && listener.publication.listeners.count() == 0)
		Script::ClearObject<LUAPublication>(_pState, listener.publication);
	else if (!done && Script::FromObject<Client>(_pState, client)) {
		LUAPublication::RemoveListener(_pState, listener.publication);
		lua_pop(_pState, 1);
	}
	Script::ClearObject<LUAListener>(_pState, listener);
}


void MonaServer::onJoinGroup(Client& client,Group& group) {
	Script::AddObject<LUAGroup>(_pState, group);
	LUAInvoker::AddGroup(_pState);
	bool done(false);
	SCRIPT_BEGIN(_pState)
		SCRIPT_MEMBER_FUNCTION_BEGIN(Client, client, "onJoinGroup")
			LUAGroup::AddClient(_pState, 1);
			done = true;
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

