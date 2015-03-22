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

#include "Mona/TCPSession.h"
#include "Mona/Protocol.h"

using namespace std;


namespace Mona {

TCPSession::TCPSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker) : _timeout(protocol.getNumber<UInt32>("timeout") * 1000), _client(peerAddress, file, invoker.sockets), Session(protocol, invoker) {
	((SocketAddress&)peer.address).set(peerAddress);


	onInitParameters = [this](const Parameters& parameters) {
		if (parameters.getNumber("timeout", _timeout))
			_timeout *= 1000;
	};

	_onError = [this](const Exception& ex) { WARN(name(), ", ", ex.error()); };

	_onData = [this](PoolBuffer& pBuffer)->UInt32 {
		if (died)
			return pBuffer->size();
		return onData(pBuffer);
	};

	_onDisconnection = [this](TCPClient& client, const SocketAddress&) { kill(SOCKET_DEATH); };

	peer.OnInitParameters::subscribe(onInitParameters);
	_client.OnError::subscribe(_onError);
	_client.OnDisconnection::subscribe(_onDisconnection);
	_client.OnData::subscribe(_onData);
}

TCPSession::~TCPSession() {
	peer.OnInitParameters::unsubscribe(onInitParameters);
	_client.OnData::unsubscribe(_onData);
	_client.OnDisconnection::unsubscribe(_onDisconnection);
	_client.OnError::unsubscribe(_onError);
}


void TCPSession::manage() {
	if (died)
		return;
	if (_timeout>0 && _client.idleTime()>_timeout) {
		kill(TIMEOUT_DEATH);
		DEBUG(name(), " timeout");
	}	
	Session::manage();
}



} // namespace Mona
