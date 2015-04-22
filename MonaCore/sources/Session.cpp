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

Session::Session(Protocol& protocol, Invoker& invoker, const shared_ptr<Peer>& pPeer, const char* name) : _sessionsOptions(0),_pPeer(pPeer),peer(*_pPeer),dumpJustInDebug(false),
	_protocol(protocol), _name(name ? name : ""), invoker(invoker), died(false), _id(0) {

	((string&)peer.protocol) = protocol.name;
	if(memcmp(peer.id,"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",ID_SIZE)==0)
		Util::Random(peer.id,ID_SIZE);

	if (!peer.serverAddress)
		((SocketAddress&)peer.serverAddress).set(protocol.publicAddress);
	else if (peer.serverAddress.port()==0)
		((SocketAddress&)peer.serverAddress).set(peer.serverAddress.host(), protocol.publicAddress.port());

	DEBUG("peer.id ", Util::FormatHex(peer.id, ID_SIZE, LOG_BUFFER));
}
	
Session::Session(Protocol& protocol, Invoker& invoker, const char* name) : _sessionsOptions(0),dumpJustInDebug(false), _pPeer(new Peer((Handler&)invoker)),
	_protocol(protocol),_name(name ? name : ""), invoker(invoker), died(false), _id(0), peer(*_pPeer) {

	((string&)peer.protocol) = protocol.name;
	if(memcmp(peer.id,"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0",ID_SIZE)==0)
		Util::Random(peer.id, ID_SIZE);

	if (!peer.serverAddress)
		((SocketAddress&)peer.serverAddress).set(protocol.publicAddress);
	else if (peer.serverAddress.port()==0)
		((SocketAddress&)peer.serverAddress).set(peer.serverAddress.host(), protocol.publicAddress.port());

	DEBUG("peer.id ", Util::FormatHex(peer.id, ID_SIZE, LOG_BUFFER));

}

Session::~Session() {
	if (!died)
		CRITIC("Session ",name()," deleted without being killed")
}

const string& Session::name() const {
	if(_name.empty())
		String::Format(_name, _protocol.name," session ",_id);
	return _name;
}

void Session::kill(UInt32 type) {
	if(died)
		return;
	peer.onDisconnection();
	(bool&)died=true;
}

void Session::DumpResponse(const char* name, const UInt8* data, UInt32 size, const SocketAddress& address, bool justInDebug) {
	// executed just in debug mode, or in dump mode
	if (!justInDebug || (justInDebug&&Logs::GetLevel() >= 7))
		DUMP(name,data, size, "Response to ", address.toString())
}

bool Session::receive(Binary& packet) {
	if(died)
		return false;
	peer.updateLastReception();
	if (!dumpJustInDebug || (dumpJustInDebug && Logs::GetLevel()>=7))
		DUMP(peer.protocol,packet.data(),packet.size(),"Request from ",peer.address.toString())
	return true;
}

bool Session::receive(const SocketAddress& address, Binary& packet) {
	// if address  has changed (possible in UDP), update it
	if (address != peer.address) {
		SocketAddress oldAddress(peer.address);
		((SocketAddress&)peer.address).set(address);
		if (id() != 0) {// id!=0 if session is managed by _sessions
			OnAddressChanged::raise(*this, oldAddress);
			if (!died)
				peer.onAddressChanged(oldAddress);
		}
	}
	return receive(packet);
}


} // namespace Mona
