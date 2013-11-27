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
#include "Mona/UDPSocket.h"
#include "Mona/RTMFP/RTMFPFlow.h"
#include "Mona/RTMFP/RTMFPWriter.h"
#include "Mona/RTMFP/RTMFPSender.h"
#include "Mona/RTMFP/RTMFPCookieComputing.h"
#include "Mona/Time.h"

namespace Mona {

class RTMFProtocol;
class RTMFPSession : public BandWriter,public Session, virtual Object {
public:

	RTMFPSession(RTMFProtocol& protocol,
			Invoker& invoker,
			UInt32 farId,
			const Peer& peer,
			const UInt8* decryptKey,
			const UInt8* encryptKey,
			const char* name=NULL);

	virtual ~RTMFPSession();

	void				p2pHandshake(const std::string& tag,const SocketAddress& address,UInt32 times,Session* pSession);

	std::shared_ptr<RTMFPCookieComputing>	pRTMFPCookieComputing;

	bool				decode(const std::shared_ptr < Buffer < UInt8 >> &pBuffer, const SocketAddress& address);

	bool				failed() const { return _failed; }

protected:
	const UInt32		farId;

	void				flush(bool echoTime = true) { flush(0x4a, echoTime, prevEngineType()); }
	void				flush(bool echoTime, RTMFPEngine::Type type) { flush(0x4a, echoTime, type); }
	void				flush(UInt8 marker, bool echoTime) { flush(marker, echoTime, prevEngineType()); }
	void				flush(UInt8 marker,bool echoTime,RTMFPEngine::Type type);


	template <typename ...Args>
	void fail(Args&&... args) {
		std::string error;
		String::Format(error, args ...);

		if (_failed)
			return;

		// Here no new sending must happen except "failSignal"
		for (auto& it : _flowWriters)
			it.second->clear();

		// unsubscribe peer for its groups
		peer.unsubscribeGroups();

		_failed = true;
		if(!error.empty()) {
			WARN("Client failed, ", error);
			failSignal();
		}
	
	}
	MemoryWriter&		writer();
	
private:

	void							manage();
	void							packetHandler(MemoryReader& packet);

	// Implementation of BandWriter
	void							initWriter(const std::shared_ptr<RTMFPWriter>& pWriter);
	std::shared_ptr<RTMFPWriter>	changeWriter(RTMFPWriter& writer);
	bool							canWriteFollowing(RTMFPWriter& writer) { return _pLastWriter == &writer; }
	void							close() { failSignal(); }

	MemoryWriter&					writeMessage(UInt8 type,UInt16 length,RTMFPWriter* pWriter=NULL);

	RTMFPEngine::Type				prevEngineType() { return _prevEngineType; }
	bool							keepAlive();
	void							kill();

	RTMFPWriter*					writer(UInt64 id);
	RTMFPFlow&						flow(UInt64 id);
	RTMFPFlow*						createFlow(UInt64 id,const std::string& signature);

	void							failSignal();

	Time											_recvTimestamp;
	UInt16											_timeSent;
	
	bool											_failed;
	UInt8											_timesFailed;
	UInt8											_timesKeepalive;

	std::map<UInt64,RTMFPFlow*>						_flows;
	RTMFPFlow*										_pFlowNull;
	std::map<UInt64,std::shared_ptr<RTMFPWriter> >	_flowWriters;
	Writer*											_pLastWriter;
	UInt64											_nextRTMFPWriterId;

	RTMFPEngine::Type								_prevEngineType;

	std::shared_ptr<RTMFPSender>					_pSender;
	UDPSocket&										_socket;

	RTMFPEngine										_decrypt;
	RTMFPEngine										_encrypt;
	PoolThread*										_pThread;
};


} // namespace Mona
