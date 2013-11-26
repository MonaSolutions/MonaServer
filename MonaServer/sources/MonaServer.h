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

#pragma once

#include "Service.h"
#include "Servers.h"
#include "Mona/Server.h"
#include "Mona/TerminateSignal.h"


class MonaServer : public Mona::Server, private ServiceRegistry, private ServerHandler {
public:
	MonaServer(Mona::TerminateSignal& terminateSignal, Mona::UInt32 bufferSize, Mona::UInt32 threads, Mona::UInt16 serversPort, const std::string& serversTarget);

	static const std::string				WWWPath;
	Servers									servers;

	bool					start(Mona::MapParameters& parameters);

private:
	void					manage();
	bool					readNextParameter(lua_State* pState, const Mona::MapParameters& parameters, const std::string& root);

	//events
	void					onStart();
	void					onStop();

	void					onRendezVousUnknown(const std::string& protocol, const Mona::UInt8* id, std::set<Mona::SocketAddress>& addresses);
	void					onHandshake(const std::string& protocol, const Mona::SocketAddress& address, const std::string& path, const std::map<std::string, std::string>& properties, Mona::UInt32 attempts, std::set<Mona::SocketAddress>& addresses);

	void					onConnection(Mona::Exception& ex, Mona::Client& client,Mona::DataReader& parameters,Mona::DataWriter& response);
	void					onFailed(const Mona::Client& client,const std::string& error);
	void					onDisconnection(const Mona::Client& client);
	void					onMessage(Mona::Exception& ex, Mona::Client& client,const std::string& name,Mona::DataReader& reader,Mona::DataWriter& writer);
	bool					onRead(Mona::Exception& ex, Mona::Client& client, std::string& filePath, Mona::DataReader& parameters);

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

	void					onManage(Mona::Client& client);

	// ServiceRegistry implementation
	void					addServiceFunction(Service& service,const std::string& name);
	void					clearService(Service& service);
	void					startService(Service& service);
	void					stopService(Service& service);

	// ServerHandler implementation
	void										connection(ServerConnection& server);
	void										message(ServerConnection& server,const std::string& handler,Mona::MemoryReader& reader);
	void										disconnection(const ServerConnection& server,const std::string& error);
	const std::string&							host() { return _host; }
	const std::map<std::string, Mona::UInt16>&		ports() { return _ports; }


	void					readLUAAddress(const std::string& protocol, std::set<Mona::SocketAddress> & addresses);
	void					readLUAAddresses(const std::string& protocol,std::set<Mona::SocketAddress>& addresses);

	lua_State*				_pState;
	Mona::TerminateSignal&	_terminateSignal;
	Service*				_pService;

	std::set<Service*>							_servicesRunning;
	std::map<std::string,std::set<Service*> >	_scriptEvents;
	std::map<std::string, Mona::UInt16>				_ports;
	std::string									_host;
};

