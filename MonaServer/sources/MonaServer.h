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

#include "Service.h"
#include "Servers.h"
#include "Mona/Server.h"
#include "Mona/TerminateSignal.h"
#include "Mona/Database.h"


class MonaServer : public Mona::Server, private ServiceHandler, private ServerHandler, private Mona::DatabaseLoader {
public:
	MonaServer(Mona::TerminateSignal& terminateSignal, Mona::UInt32 bufferSize, Mona::UInt16 threads, Mona::UInt16 serversPort, const std::string& serversTarget);

	static const std::string				WWWPath;
	static const std::string				DataPath;

	Servers									servers;

	bool					start(Mona::MapParameters& parameters);

private:
	void					manage();
	void					createParametersCollection(const char* name, const Mona::MapParameters& parameters);

	void					readLUAAddress(const std::string& protocol, std::set<Mona::SocketAddress> & addresses);
	void					readLUAAddresses(const std::string& protocol, std::set<Mona::SocketAddress>& addresses);

	lua_State*				openService(const Mona::Client& client);

	// DatabaseLoader implementation
	void					onDataLoading(const std::string& path, const char* value, Mona::UInt32 size);

	// ServiceHandler implementation
	void					startService(Service& service);
	void					stopService(Service& service);


	// ServerHandler implementation
	void										connection(ServerConnection& server);
	void										message(ServerConnection& server, const std::string& handler, Mona::MemoryReader& reader);
	void										disconnection(const ServerConnection& server, const std::string& error);
	const std::string&							host() { return _host; }
	const std::map<std::string, Mona::UInt16>&	ports() { return _ports; }

	//events
	void					onStart();
	void					onStop();

	void					onRendezVousUnknown(const std::string& protocol, const Mona::UInt8* id, std::set<Mona::SocketAddress>& addresses);
	void					onHandshake(const std::string& protocol, const Mona::SocketAddress& address, const std::string& path, const Mona::MapParameters& properties, Mona::UInt32 attempts, std::set<Mona::SocketAddress>& addresses);

	void					onConnection(Mona::Exception& ex, Mona::Client& client,Mona::DataReader& parameters,Mona::DataWriter& response);
	void					onDisconnection(const Mona::Client& client);
	void					onMessage(Mona::Exception& ex, Mona::Client& client,const std::string& name,Mona::DataReader& reader,Mona::UInt8 responseType);
	bool					onRead(Mona::Exception& ex, Mona::Client& client, Mona::FilePath& filePath, Mona::DataReader& parameters,Mona::DataWriter& properties);

	void					onJoinGroup(Mona::Client& client,Mona::Group& group);
	void					onUnjoinGroup(Mona::Client& client,Mona::Group& group);

	bool					onPublish(Mona::Client& client,const Mona::Publication& publication,std::string& error);
	void					onUnpublish(Mona::Client& client,const Mona::Publication& publication);

	void					onAudioPacket(Mona::Client& client, const Mona::Publication& publication, Mona::UInt32 time, Mona::MemoryReader& packet);
	void					onVideoPacket(Mona::Client& client, const Mona::Publication& publication, Mona::UInt32 time, Mona::MemoryReader& packet);
	void					onDataPacket(Mona::Client& client,const Mona::Publication& publication,Mona::DataReader& packet);
	void					onFlushPackets(Mona::Client& client,const Mona::Publication& publication);

	bool					onSubscribe(Mona::Client& client,const Mona::Listener& listener,std::string& error);
	void					onUnsubscribe(Mona::Client& client,const Mona::Listener& listener);


	lua_State*					_pState;
	Mona::TerminateSignal&		_terminateSignal;
	std::unique_ptr<Service>	_pService;

	std::set<Service*>					_servicesRunning;
	std::map<std::string, Mona::UInt16>	_ports;
	std::string							_host;
	Mona::Database						_data;
	std::string							_buffer;
};

