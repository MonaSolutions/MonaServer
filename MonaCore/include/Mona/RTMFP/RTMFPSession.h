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
#include "Mona/RTMFP/RTMFPDecoder.h"
#include "Mona/Time.h"

namespace Mona {

class RTMFProtocol;
class RTMFPSession : public BandWriter,public Session, public virtual Object {
public:

	RTMFPSession(RTMFProtocol& protocol,
			Invoker& invoker,
			UInt32 farId,
			const std::shared_ptr<RTMFPEngine> pDecoder,
			const std::shared_ptr<RTMFPEngine> pEncoder,
			const std::shared_ptr<Peer>& pPeer);


	bool				p2pHandshake(const std::string& tag,const SocketAddress& address,UInt32 times,Session* pSession);

	std::shared_ptr<RTMFPCookieComputing>	pRTMFPCookieComputing;

	void				decode(const SocketAddress& address, PoolBuffer& pBuffer) { if(!died) _decoder.decode(address,pBuffer);  }

	bool				failed() const { return _failed; }
	void				kill(UInt32 type=NORMAL_DEATH);

protected:
	RTMFPSession(RTMFProtocol& protocol,
			Invoker& invoker,
			UInt32 farId,
			const UInt8* decryptKey,
			const UInt8* encryptKey,
			const char* name);

	const UInt32		farId;

	

	UInt8*				packet(); // size = RTMFP_MAX_PACKET_SIZE
	void				flush(UInt8 marker, UInt32 size);

private:

	template <typename ...Args>
	void fail(Args&&... args) {
		if (_failed)
			return;

		releasePeer();

		std::string error;
		String::Format(error, args ...);
		if (!error.empty())
			WARN("Client failed, ", error);
		failSignal();
	}
	
	virtual void					receive(const SocketAddress& address, BinaryReader& packet);
	void							flush(bool echoTime, UInt8 marker = 0x4a);
	void							manage();

	// Implementation of BandWriter
	void							flush() { flush(true); }
	UInt16							ping() const				{ return peer.ping(); }
	const PoolBuffers&				poolBuffers() { return invoker.poolBuffers; }
	void							initWriter(const std::shared_ptr<RTMFPWriter>& pWriter);
	std::shared_ptr<RTMFPWriter>	changeWriter(RTMFPWriter& writer);
	bool							canWriteFollowing(RTMFPWriter& writer) { return _pLastWriter == &writer; }
	UInt32							availableToWrite() { return RTMFP_MAX_PACKET_SIZE - (_pSender ? _pSender->packet.size() : RTMFP_HEADER_SIZE); }
	BinaryWriter&					writeMessage(UInt8 type,UInt16 length,RTMFPWriter* pWriter=NULL);

	void							writeP2PHandshake(const std::string& tag, const SocketAddress& address, RTMFP::AddressType type);

	bool							keepalive();

	RTMFPWriter*					writer(UInt64 id);
	RTMFPFlow*						createFlow(UInt64 id,const std::string& signature);

	void							failSignal();
	void							releasePeer();

	FlashStream::OnStart::Type						onStreamStart;
	FlashStream::OnStop::Type						onStreamStop;

	RTMFPDecoder::OnDecoded::Type					onDecoded;
	RTMFPDecoder::OnDecodedEnd::Type				onDecodedEnd;

	RTMFPDecoder									_decoder;
	const std::shared_ptr<RTMFPEngine>				_pEncoder;

	UInt16											_timeSent;
	bool											_failed;
	UInt8											_timesFailed;
	UInt8											_timesKeepalive;

	std::shared_ptr<FlashMainStream>				_pMainStream;
	std::map<UInt64,RTMFPFlow*>						_flows;
	RTMFPFlow*										_pFlowNull;
	std::map<UInt64,std::shared_ptr<RTMFPWriter> >	_flowWriters;
	Writer*											_pLastWriter;
	UInt64											_nextRTMFPWriterId;

	std::shared_ptr<RTMFPSender>					_pSender;
	PoolThread*										_pThread;
};


} // namespace Mona
