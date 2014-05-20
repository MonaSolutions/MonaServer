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

TCPSession::TCPSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker) : _timeout(protocol.getNumber<UInt32>("timeout")*1000), _client(peerAddress, file, invoker.sockets), Session(protocol, invoker), _consumed(false), _decoding(false) {
	((SocketAddress&)peer.address).set(peerAddress);

	onInitParameters = [this](const Parameters& parameters) {
		if (parameters.getNumber("timeout", _timeout))
			_timeout *= 1000;
	};

	onError = [this](const Exception& ex) { WARN("Protocol ", this->protocol().name, ", ", ex.error()); };

	onData = [this](PoolBuffer& pBuffer)->UInt32 {
		if (died)
			return 0;
		UInt32 size(pBuffer->size());
		PacketReader packet(pBuffer->data(), size);
		_decoding = false;
		if (!buildPacket(pBuffer,packet)) {
			if (!_decoding && _consumed) {
				flush(); // flush
				_consumed = false;
			}
			return size;
		}

		UInt32 rest = size - packet.position() - packet.available();
		if (!_decoding) {
			receiveWithoutFlush(packet);
			if (rest == 0) {
				flush(); // flush
				_consumed = false;
			} else
				_consumed = true;
		}
		return rest;
	};

	onDisconnection = [this]() { kill(SOCKET_DEATH); };

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
	receiveWithoutFlush(packet);
	bool empty = _decodings.empty();
	while (!_decodings.empty() && _decodings.front().unique())
		_decodings.pop_front();
	if (!_decodings.empty())
		_decodings.pop_front();
	if (!empty && _decodings.empty()) {
		flush();
		_consumed = true;
	}
}

void TCPSession::manage() {
	if (died)
		return;
	Session::manage();
	if (_timeout>_client.idleTime()) {
		kill(TIMEOUT_DEATH);
		DEBUG(protocol().name, " timeout session ", name());
	}	
}



} // namespace Mona
