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
#include "Mona/Group.h"
#include "Mona/Publication.h"
#include "Mona/ICE.h"
#include "Mona/File.h"
#include <set>

namespace Mona {

namespace Events {
	struct OnInitParameters : Event<void(const Parameters&)> {};
};


class Handler;
class Peer : public Client, public virtual Object,
	public Events::OnInitParameters {
public:

	Peer(Handler& handler);
	virtual ~Peer();

	Writer&						writer() { return _pWriter ? *_pWriter : Writer::Null; }

	std::set<SocketAddress>		localAddresses;
	const bool					connected;

	const Parameters&			parameters() const { return _parameters; }
	const Parameters&			properties() const { return _properties; }
	MapParameters&				properties() { return _properties; }

	void						setPath(const std::string& value) { ((std::string&)Client::path).assign(value); }
	void						setQuery(const std::string& value) { ((std::string&)Client::query).assign(value); }
	void						setServerAddress(const std::string& address);

	template<typename PingType>
	void						setPing(PingType value) { _ping = (value>0xFFFF ? 0xFFFF : (value==0 ? 1 : (UInt16)value)); _pingProcessing = false; _pingTime.update(); }
	UInt16						ping() const;
	bool						ping(UInt32 obsoleteDelay);
	void						pong();

	void						updateLastReception() { ((Time&)lastReceptionTime).update();  }
	
	ICE&		ice(const Peer& peer);

	void	unsubscribeGroups(const std::function<void(const Group& group)>& forEach=nullptr);
	Group&  joinGroup(const UInt8* id, Writer* pWriter);
	void    unjoinGroup(Group& group);


	// events
	void onConnection(Exception& ex, Writer& writer, DataReader& parameters) { onConnection(ex, writer, parameters, DataWriter::Null); }
	void onConnection(Exception& ex, Writer& writer, DataReader& parameters, DataWriter& response);
	void onDisconnection();

	void onRendezVousUnknown(const UInt8* peerId, std::set<SocketAddress>& addresses);
	void onHandshake(UInt32 attempts, std::set<SocketAddress>& addresses);
	void onAddressChanged(const SocketAddress& oldAddress);

	bool onPublish(Exception& ex, const Publication& publication);
	void onUnpublish(const Publication& publication);

	bool onSubscribe(Exception& ex, const Listener& listener);
	void onUnsubscribe(const Listener& listener);

	bool onMessage(Exception& ex, const std::string& name, DataReader& reader, UInt8 responseType = 0);
	/// \brief call the onRead lua function ang get result in properties
	/// \param filePath : relative path to the file (important : the directory will be erase)
	/// \param parameters : gives parameters to the function onRead()
	/// \param properties : recieve output parameters returned by onRead()
	bool onRead(Exception& ex, DataReader& parameters, File& file, DataWriter& properties) { return onFileAccess(ex, FileAccessType::READ, parameters, file, properties); }
	bool onWrite(Exception& ex, DataReader& parameters, File& file, DataWriter& properties) { return onFileAccess(ex, FileAccessType::WRITE, parameters, file, properties); }

private:
	bool onFileAccess(Exception& ex, FileAccessType type, DataReader& parameters, File& file, DataWriter& properties);

	void onJoinGroup(Group& group);
	void onUnjoinGroup(Group& group,bool dummy);
	bool exchangeMemberId(Group& group,Peer& peer,Writer* pWriter);

	Handler&						_handler;
	std::map<Group*,Writer*>		_groups;
	std::map<const Peer*,ICE*>		_ices;
	MapParameters					_parameters;
	MapParameters					_properties;

	UInt16							_ping;
	Time							_pingTime;
	bool							_pingProcessing;
	Writer*							_pWriter;
};

} // namespace Mona
