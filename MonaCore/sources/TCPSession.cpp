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

	_receptions.emplace_back(0);

	onInitParameters = [this](const Parameters& parameters) {
		if (parameters.getNumber("timeout", _timeout))
			_timeout *= 1000;
	};

	onError = [this](const Exception& ex) { WARN(name(), ", ", ex.error()); };

	onData = [this](PoolBuffer& pBuffer)->UInt32 {
		if (died)
			return 0;
		UInt32 size(pBuffer->size());
		PacketReader packet(pBuffer->data(), size);
		UInt32 receptions(_receptions.back());
		if (!buildPacket(pBuffer,packet)) {
			if (_receptions.size()>1 && _receptions.front() > 0 && (--_receptions.front()) == 0) {
				_receptions.pop_front();
				flush();
			}
			return size;
		}

		bool noDecoding(receptions == _receptions.back());
	
		UInt32 rest = size - packet.position() - packet.available();
		if (rest==0)
			_receptions.emplace_back(0);

		if (noDecoding) {
			++_receptions.back();
			receive(packet);
		}

		return rest;
	};

	onDisconnection = [this](const SocketAddress&) { kill(SOCKET_DEATH); };

	peer.OnInitParameters::subscribe(onInitParameters);
	_client.OnError::subscribe(onError);
	_client.OnDisconnection::subscribe(onDisconnection);
	_client.OnData::subscribe(onData);
}

TCPSession::~TCPSession() {
	peer.OnInitParameters::unsubscribe(onInitParameters);
	_client.OnData::unsubscribe(onData);
	_client.OnDisconnection::unsubscribe(onDisconnection);
	_client.OnError::unsubscribe(onError);
}


void TCPSession::receive(PacketReader& packet) {
	Session::receive(packet);
	if (_receptions.size()>1 && _receptions.front() > 0 && (--_receptions.front()) == 0) {
		_receptions.pop_front();
		return flush();
	}
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
