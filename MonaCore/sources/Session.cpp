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

#include "Mona/Session.h"
#include "Mona/Protocol.h"
#include "Mona/Sessions.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"

using namespace std;


namespace Mona {

Session::Session(Protocol& protocol, Invoker& invoker, const Peer& peer, const char* name) : _pSessions(NULL), dumpJustInDebug(false),
	Expirable(this), _protocol(protocol), _name(name ? name : ""), invoker(invoker), _pDecodingThread(NULL), died(false), _id(0), peer(peer) {
	this->peer.setString("protocol", protocol.name);
	if(memcmp(this->peer.id,"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",ID_SIZE)==0)
		Util::Random(this->peer.id,ID_SIZE);
	string hex;
	DEBUG("peer.id: ", Util::FormatHex(this->peer.id, ID_SIZE, hex));
}
	
Session::Session(Protocol& protocol, Invoker& invoker, const char* name) : dumpJustInDebug(false), _pSessions(NULL),
	Expirable(this),_protocol(protocol),_name(name ? name : ""), invoker(invoker), _pDecodingThread(NULL), died(false), _id(0), peer((Handler&)invoker) {
	peer.setString("protocol", protocol.name);
	if(memcmp(peer.id,"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",ID_SIZE)==0)
		Util::Random(this->peer.id, ID_SIZE);
	string hex;
	DEBUG("peer.id: ", Util::FormatHex(this->peer.id, ID_SIZE, hex));
}


Session::~Session() {
	kill();
	expire();
}

const string& Session::name() const {
	if(_name.empty())
		String::Format(_name, _id);
	return _name;
}

void Session::kill() {
	if(died)
		return;
	peer.onDisconnection();
	(bool&)died=true;
}

void Session::receiveWithoutFlush(PacketReader& packet) {
	if(died)
		return;
	if (!dumpJustInDebug || (dumpJustInDebug && Logs::GetLevel()>=7))
		DUMP(packet.data(),packet.size(),"Request from ",peer.address.toString())
	packetHandler(packet);
}

void Session::receive(PacketReader& packet, const SocketAddress& address) {
	// if address  has changed (possible in UDP), update it
	if (address != peer.address) {
		SocketAddress oldAddress(peer.address);
		((SocketAddress&)peer.address).set(address);
		if (_pSessions && id() != 0) // id!=0 if session is managed by _sessions
			_pSessions->updateAddress(*this, oldAddress);
	}
	receive(packet);
}

const string&  Session::protocolName() {
	return _protocol.name;
}


} // namespace Mona
