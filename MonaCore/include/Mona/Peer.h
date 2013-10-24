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

#pragma once

#include "Mona/Mona.h"
#include "Mona/Exceptions.h"
#include "Mona/Client.h"
#include "Mona/DataReader.h"
#include "Mona/ICE.h"
#include <set>

namespace Mona {


class Group;
class Handler;
class Publication;
class Listener;
class Member;
class Peer : public Client {
public:
	Peer(Handler& handler);
	Peer(const Peer& peer);
	virtual ~Peer();

	std::list<SocketAddress>	addresses;
	const bool					connected;

	Entities<Client>::Map		turnPeers;
	bool						relayable;

	bool		setName(const std::string& name);

	ICE&		ice(const Peer& peer);

	void unsubscribeGroups();
	void joinGroup(Group& group,Writer* pWriter);
	Group& joinGroup(const Poco::UInt8* id,Writer* pWriter);
	void unjoinGroup(Group& group);


// events
	void onRendezVousUnknown(const Poco::UInt8* peerId,std::set<SocketAddress>& addresses);
	void onHandshake(Poco::UInt32 attempts,std::set<SocketAddress>& addresses);

	void onConnection(Exception& ex, Writer& writer) {onConnection(ex, writer,DataReaderNull,Writer::DataWriterNull);}
	void onConnection(Exception& ex, Writer& writer,DataWriter& response) {onConnection(ex, writer,DataReaderNull,response);}
	void onConnection(Exception& ex, Writer& writer,DataReader& parameters,DataWriter& response);
	void onFailed(const std::string& error);
	void onDisconnection();
	void onMessage(Exception& ex, const std::string& name,DataReader& reader);

	bool onPublish(const Publication& publication,std::string& error);
	void onUnpublish(const Publication& publication);

	void onDataPacket(const Publication& publication,DataReader& packet);
	void onAudioPacket(const Publication& publication,Poco::UInt32 time,MemoryReader& packet);
	void onVideoPacket(const Publication& publication,Poco::UInt32 time,MemoryReader& packet);
	void onFlushPackets(const Publication& publication);

	bool onSubscribe(const Listener& listener,std::string& error);
	void onUnsubscribe(const Listener& listener);

	bool onRead(std::string& filePath) { return onRead(filePath, MapParameters()); }
	bool onRead(std::string& filePath,MapParameters& parameters);

	void onManage();

	static DataReaderNull	DataReaderNull;
private:
	void onJoinGroup(Group& group);
	void onUnjoinGroup(std::map<Group*,Member*>::iterator it);
	bool writeId(Group& group,Peer& peer,Writer* pWriter);

	Handler&						_handler;
	std::map<Group*,Member*>		_groups;
	std::map<const Peer*,ICE*>		_ices;
};

} // namespace Mona
