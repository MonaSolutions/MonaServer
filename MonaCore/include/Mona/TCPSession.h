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
#include "Mona/Session.h"
#include "Mona/TCPClient.h"

namespace Mona {

class TCPSession : public Session, public virtual Object {
public:
	
	void kill(UInt32 type=NORMAL_DEATH) { _client.disconnect(); Session::kill(type); }

	template<typename TCPSenderType>
	bool send(Exception& ex,const QualityOfService& qos,const std::shared_ptr<TCPSenderType>& pSender) {
		((QualityOfService&)qos).add(pSender->size(),peer.ping());
		return _client.send<TCPSenderType>(ex, pSender);
	}
	template<typename TCPSenderType>
	PoolThread*	send(Exception& ex,const QualityOfService& qos,const std::shared_ptr<TCPSenderType>& pSender, PoolThread* pThread) {
		((QualityOfService&)qos).add(pSender->size(),peer.ping());
		return _client.send<TCPSenderType>(ex, pSender, pThread);
	}

protected:
	TCPSession(const SocketAddress& peerAddress, SocketFile& file,Protocol& protocol,Invoker& invoker);
	virtual ~TCPSession();

	void			manage();

	template<typename TCPSenderType>
	bool send(Exception& ex,const std::shared_ptr<TCPSenderType>& pSender) { return _client.send<TCPSenderType>(ex, pSender); }
	template<typename TCPSenderType>
	PoolThread*	send(Exception& ex,const std::shared_ptr<TCPSenderType>& pSender, PoolThread* pThread) { return _client.send<TCPSenderType>(ex, pSender, pThread); }

private:

	virtual UInt32	onData(PoolBuffer& pBuffer) = 0;

	// TCPClient events
	TCPClient::OnError::Type			_onError;
	TCPClient::OnData::Type				_onData;
	TCPClient::OnDisconnection::Type	_onDisconnection;
	Peer::OnInitParameters::Type		onInitParameters;


	UInt32				_timeout;
	TCPClient			_client;
};



} // namespace Mona
