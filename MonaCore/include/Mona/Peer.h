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

#include "Mona/Mona.h"
#include "Mona/ICE.h"
#include "Mona/Exceptions.h"
#include "Mona/Client.h"
#include "Mona/DataReader.h"
#include "Mona/FilePath.h"
#include <set>

namespace Mona {

namespace Events {
	struct OnInitParameters : Event<void(const Parameters&)> {};
};


class Group;
class Handler;
class Publication;
class Listener;
class Member;
class Peer : public Client, public virtual Object,
	public Events::OnInitParameters {
public:
	Peer(Handler& handler);
	virtual ~Peer();

	std::set<SocketAddress>		localAddresses;
	const bool					connected;

	const Parameters&			parameters() const { return _parameters; }
	const Parameters&			properties() const { return _properties; }
	MapParameters&				properties() { return _properties; }
	
	void						setPath(const std::string& value) { ((std::string&)Client::path).assign(value); }
	void						setQuery(const std::string& value) { ((std::string&)Client::query).assign(value); }
	void						setServerAddress(const std::string& value) { ((std::string&)Client::serverAddress).assign(value); }
	void						setPing(UInt16 value) { ((UInt16&)ping) = value; }

	ICE&		ice(const Peer& peer);

	void unsubscribeGroups();
	Group& joinGroup(const UInt8* id,Writer* pWriter);
	void unjoinGroup(Group& group);


// events
	void onRendezVousUnknown(const UInt8* peerId,std::set<SocketAddress>& addresses);
	void onHandshake(UInt32 attempts,std::set<SocketAddress>& addresses);

	void onConnection(Exception& ex, Writer& writer, DataReader& parameters) { onConnection(ex, writer, parameters, DataWriter::Null); }
	void onConnection(Exception& ex, Writer& writer,DataReader& parameters,DataWriter& response);
	void onDisconnection();
	bool onMessage(Exception& ex, const std::string& name,DataReader& reader, UInt8 responseType=0);

	bool onPublish(const Publication& publication,std::string& error);
	void onUnpublish(const Publication& publication);

	void onDataPacket(const Publication& publication,DataReader& packet);
	void onAudioPacket(const Publication& publication,UInt32 time,PacketReader& packet);
	void onVideoPacket(const Publication& publication,UInt32 time,PacketReader& packet);
	void onFlushPackets(const Publication& publication);

	bool onSubscribe(const Listener& listener,std::string& error);
	void onUnsubscribe(const Listener& listener);

	/// \brief call the onRead lua function ang get result in properties
	/// \param filePath : relative path to the file (important : the directory will be erase)
	/// \param parameters : gives parameters to the function onRead()
	/// \param properties : recieve output parameters returned by onRead()
    bool onRead(Exception& ex, FilePath& filePath, DataReader& parameters,DataWriter& properties);

private:
	void onJoinGroup(Group& group);
	void onUnjoinGroup(Group& group,bool dummy);
	bool exchangeMemberId(Group& group,Peer& peer,Writer* pWriter);

	Handler&						_handler;
	std::map<Group*,Writer*>		_groups;
	std::map<const Peer*,ICE*>		_ices;
	MapParameters					_parameters;
	MapParameters					_properties;
	
};

} // namespace Mona
