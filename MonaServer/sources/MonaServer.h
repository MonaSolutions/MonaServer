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
#include "Servers.h"
#include "Service.h"
#include "Mona/PersistentData.h"


class MonaServer : public Mona::Server, private ServiceHandler {
public:
	MonaServer(const Mona::Parameters& configs, Mona::TerminateSignal& terminateSignal);
	~MonaServer();

	Servers					servers;

	bool					start(const Mona::Parameters& configs);

private:
	void					manage();


	void					readAddressRedirection(const std::string& protocol, int& index, std::set<Mona::SocketAddress> & addresses);


	lua_State*				openService(const Service& service, Mona::Client& client);
	lua_State*				loadService(const Mona::Client& client) { return (_pState && client.hasCustomData()) ? _pState : NULL; }
	lua_State*				closeService(const Mona::Client& client, int& reference);

	/// ServiceHandler implementation
	void					startService(Service& service);
	void					stopService(Service& service);

	/// Server implementation
	const std::string&		rootPath() const { return _wwwPath; }

	//events
	void					onStart();
	void					onStop();

	void					onRendezVousUnknown(const std::string& protocol, const Mona::UInt8* id, std::set<Mona::SocketAddress>& addresses);
	void					onHandshake(const std::string& protocol, const Mona::SocketAddress& address, const std::string& path, const Mona::Parameters& properties, Mona::UInt32 attempts, std::set<Mona::SocketAddress>& addresses);

	void					onConnection(Mona::Exception& ex, Mona::Client& client,Mona::DataReader& parameters,Mona::DataWriter& response);
	void					onDisconnection(const Mona::Client& client);
	void					onAddressChanged(Mona::Client& client, const Mona::SocketAddress& oldAddress);
	bool					onMessage(Mona::Exception& ex, Mona::Client& client,const std::string& name,Mona::DataReader& reader,Mona::UInt8 responseType);
	bool					onFileAccess(Mona::Exception& ex, Mona::Client& client, Mona::Client::FileAccessType type, Mona::DataReader& parameters, Mona::File& file, Mona::DataWriter& properties);

	void					onJoinGroup(Mona::Client& client,Mona::Group& group);
	void					onUnjoinGroup(Mona::Client& client,Mona::Group& group);

	bool					onPublish(Mona::Exception& ex,const Mona::Publication& publication, Mona::Client* pClient);
	void					onUnpublish(const Mona::Publication& publication, Mona::Client* pClient);

	bool					onSubscribe(Mona::Exception& ex, Mona::Client& client,const Mona::Listener& listener);
	void					onUnsubscribe(Mona::Client& client,const Mona::Listener& listener);

	Servers::OnConnection::Type		onServerConnection;
	Servers::OnMessage::Type		onServerMessage;
	Servers::OnDisconnection::Type	onServerDisconnection;

	Mona::Publication::OnData::Type			onPublicationData;
	Mona::Publication::OnAudio::Type		onPublicationAudio;
	Mona::Publication::OnVideo::Type		onPublicationVideo;
	Mona::Publication::OnFlush::Type		onPublicationFlush;
	Mona::Publication::OnProperties::Type	onPublicationProperties;


	Mona::Parameters::ForEach	setLUAProperty;

	lua_State*					_pState;
	Mona::TerminateSignal&		_terminateSignal;
	std::unique_ptr<Service>	_pService;

	std::set<Service*>			_servicesRunning;
	Mona::PersistentData		_data;

	std::string					_wwwPath;
	std::string					_dataPath;
};

