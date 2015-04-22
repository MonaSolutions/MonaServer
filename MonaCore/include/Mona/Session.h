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
#include "Mona/Peer.h"
#include "Mona/Invoker.h"
#include "Mona/Logs.h"

namespace Mona {

class Session;
namespace Events {
	struct OnAddressChanged : Event<void(Session&,const SocketAddress&)> {};
};

class Protocol;
class Session : public virtual Object,
	public Events::OnAddressChanged {
	friend class Sessions;

private:
	const std::shared_ptr<Peer> _pPeer; // before "peer" member to be created before it!!

public:
	enum {
		NORMAL_DEATH=0,
		// 0xFFFFFFX to allow code protocol, like HTTP with value already conventionned
		SERVER_DEATH=0xFFFFFFFB, 
		SOCKET_DEATH=0xFFFFFFFC,
		TIMEOUT_DEATH=0xFFFFFFFD,
		REJECTED_DEATH=0xFFFFFFFE,
		PROTOCOL_DEATH=0xFFFFFFFF
	};

	virtual ~Session();

	UInt32				id()	const { return _id; }
	const std::string&	name()	const;
	
	template<typename ProtocolType = Protocol>
	ProtocolType& protocol() { return (ProtocolType&)_protocol; }

	Peer&				peer;
	Invoker&			invoker;

	const bool			died;
	
	bool				dumpJustInDebug;


	static void	DumpResponse(const char* name, const UInt8* data, UInt32 size, const SocketAddress& address, bool justInDebug = false);
	void dumpResponse(const UInt8* data, UInt32 size, bool justInDebug=false) { DumpResponse(peer.protocol.c_str(), data, size, peer.address, justInDebug); }


	template<typename ProtocolType,typename SenderType>
	bool send(Exception& ex,const std::shared_ptr<SenderType>& pSender) {
		return ((ProtocolType&)_protocol).template send<SenderType>(ex, pSender);
	}
	template<typename ProtocolType,typename SenderType>
	PoolThread*	send(Exception& ex,const std::shared_ptr<SenderType>& pSender, PoolThread* pThread) {
		return ((ProtocolType&)_protocol).template send<SenderType>(ex, pSender, pThread);
	}

	virtual void		manage() { flush(); }
	virtual void		kill(UInt32 type=NORMAL_DEATH);
	virtual void		flush() { peer.writer().flush(); }

protected:
	Session(Protocol& protocol,Invoker& invoker, const char* name=NULL);
	Session(Protocol& protocol, Invoker& invoker, const std::shared_ptr<Peer>& pPeer, const char* name = NULL);

	bool receive(Binary& packet);
	bool receive(const SocketAddress& address, Binary& packet);

private:
	
	mutable std::string					_name;
	UInt32								_id;
	UInt8								_sessionsOptions;
	Protocol&							_protocol;
};


} // namespace Mona
