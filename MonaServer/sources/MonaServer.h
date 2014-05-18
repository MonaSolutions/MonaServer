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

#include "Mona/Server.h"
#include "Mona/TerminateSignal.h"
#include "Mona/Database.h"
#include "Servers.h"
#include "Service.h"


class MonaServer : public Mona::Server, private ServiceHandler, private Mona::DatabaseLoader {
public:
	MonaServer(Mona::TerminateSignal& terminateSignal, const Mona::Parameters& configs);
	~MonaServer();

	static const std::string				WWWPath;
	static const std::string				DataPath;

	Servers									servers;

	bool					start();

private:
	void					manage();

	
	void					readLUAAddresses(const std::string& protocol, std::set<Mona::SocketAddress>& addresses);
	void					readLUAAddress(const std::string& protocol, std::set<Mona::SocketAddress> & addresses);
	void					readLUAServerAddress(const std::string& protocol, std::set<Mona::SocketAddress>& addresses,const ServerConnection& server);

	template<typename ...Args>
	void addLUAAddress(std::set<Mona::SocketAddress>& addresses, Args&&... args) {
		Mona::SocketAddress address;
		Mona::Exception ex;
		if (address.set(ex, args ...)) {
			addresses.emplace(address);
			if (ex) {
				SCRIPT_BEGIN(_pState)
					SCRIPT_WARN(ex.error());
				SCRIPT_END
			}
		} else if (ex) {
			SCRIPT_BEGIN(_pState)
				SCRIPT_ERROR("Invalid address, ", ex.error());
			SCRIPT_END
		}
	}

	lua_State*				openService(const Mona::Client& client);

	// DatabaseLoader implementation
	void					onDataLoading(const std::string& path, const Mona::UInt8* value, Mona::UInt32 size);

	// ServiceHandler implementation
	void					startService(Service& service);
	void					stopService(Service& service);
	const std::string&		wwwPath() { return WWWPath; }



	//events
	void					onStart();
	void					onStop();

	void					onRendezVousUnknown(const std::string& protocol, const Mona::UInt8* id, std::set<Mona::SocketAddress>& addresses);
	void					onHandshake(const std::string& protocol, const Mona::SocketAddress& address, const std::string& path, const Mona::Parameters& properties, Mona::UInt32 attempts, std::set<Mona::SocketAddress>& addresses);

	void					onConnection(Mona::Exception& ex, Mona::Client& client,Mona::DataReader& parameters,Mona::DataWriter& response);
	void					onDisconnection(const Mona::Client& client);
	bool					onMessage(Mona::Exception& ex, Mona::Client& client,const std::string& name,Mona::DataReader& reader,Mona::UInt8 responseType);
	bool					onRead(Mona::Exception& ex, Mona::Client& client, Mona::FilePath& filePath, Mona::DataReader& parameters,Mona::DataWriter& properties);

	void					onJoinGroup(Mona::Client& client,Mona::Group& group);
	void					onUnjoinGroup(Mona::Client& client,Mona::Group& group);

	bool					onPublish(Mona::Client& client,const Mona::Publication& publication,std::string& error);
	void					onUnpublish(Mona::Client& client,const Mona::Publication& publication);

	void					onAudioPacket(Mona::Client& client, const Mona::Publication& publication, Mona::UInt32 time, Mona::PacketReader& packet);
	void					onVideoPacket(Mona::Client& client, const Mona::Publication& publication, Mona::UInt32 time, Mona::PacketReader& packet);
	void					onDataPacket(Mona::Client& client,const Mona::Publication& publication,Mona::DataReader& packet);
	void					onFlushPackets(Mona::Client& client,const Mona::Publication& publication);

	bool					onSubscribe(Mona::Client& client,const Mona::Listener& listener,std::string& error);
	void					onUnsubscribe(Mona::Client& client,const Mona::Listener& listener);

	Servers::OnConnection::Type		onServerConnection;
	Servers::OnMessage::Type		onServerMessage;
	Servers::OnDisconnection::Type	onServerDisconnection;

	Parameters::ForEach			setLUAProperty;

	lua_State*					_pState;
	Mona::TerminateSignal&		_terminateSignal;
	std::unique_ptr<Service>	_pService;

	std::set<Service*>			_servicesRunning;
	Mona::Database				_data;
	bool						_firstData;

	
};

