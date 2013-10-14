/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
#include "Mona/RTMFP/RTMFPFlow.h"
#include "Mona/RTMFP/RTMFPWriter.h"
#include "Mona/RTMFP/RTMFPSender.h"
#include "Mona/Time.h"

namespace Mona {

class RTMFProtocol;
class RTMFPSession : public BandWriter,public Session {
public:

	RTMFPSession(RTMFProtocol& protocol,
			Invoker& invoker,
			Mona::UInt32 farId,
			const Peer& peer,
			const Mona::UInt8* decryptKey,
			const Mona::UInt8* encryptKey,
			const char* name=NULL);

	virtual ~RTMFPSession();

	void				p2pHandshake(const std::string& tag,const Poco::Net::SocketAddress& address,Mona::UInt32 times,Session* pSession);

protected:
	const Mona::UInt32	farId;

	void				flush(bool echoTime=true);
	void				flush(bool echoTime,RTMFPEngine::Type type);
	void				flush(Mona::UInt8 marker,bool echoTime);
	void				flush(Mona::UInt8 marker,bool echoTime,RTMFPEngine::Type type);

	void				fail(const std::string& fail);
	MemoryWriter&		writer();
	
private:

	void				manage();
	MemoryReader*		decode(Poco::SharedPtr<Poco::Buffer<Mona::UInt8> >& pBuffer,const Poco::Net::SocketAddress& address);
	void				packetHandler(MemoryReader& packet);

	// Implementation of BandWriter
	void				initWriter(RTMFPWriter& writer);
	void				resetWriter(RTMFPWriter& writer);
	bool				canWriteFollowing(RTMFPWriter& writer);
	void				close();
	bool				failed() const;

	MemoryWriter&		writeMessage(Mona::UInt8 type,Mona::UInt16 length,RTMFPWriter* pWriter=NULL);

	RTMFPEngine::Type	prevEngineType();
	bool				keepAlive();
	void				kill();

	RTMFPWriter*			writer(Mona::UInt64 id);
	RTMFPFlow&				flow(Mona::UInt64 id);
	RTMFPFlow*				createFlow(Mona::UInt64 id,const std::string& signature);

	void				failSignal();

	Mona::Time				_recvTimestamp; // Protected for Middle session
	Mona::UInt16				_timeSent; // Protected for Middle session
	
	Mona::UInt8							_timesFailed;
	Mona::UInt8							_timesKeepalive;

	std::map<Mona::UInt64,RTMFPFlow*>						_flows;
	RTMFPFlow*												_pFlowNull;
	std::map<Mona::UInt64,Poco::AutoPtr<RTMFPWriter> >	_flowWriters;
	Writer*												_pLastWriter;
	Mona::UInt64										_nextRTMFPWriterId;

	RTMFPEngine::Type					_prevEngineType;

	Poco::AutoPtr<RTMFPSender>			 _pSender;

	RTMFPEngine							_decrypt;
	RTMFPEngine							_encrypt;
	PoolThread*							_pThread;
};

inline RTMFPEngine::Type RTMFPSession::prevEngineType() {
	return _prevEngineType;
}

inline void RTMFPSession::close() {
	failSignal();
}

inline void RTMFPSession::flush(Mona::UInt8 marker,bool echoTime) {
	flush(marker,echoTime,prevEngineType());
}

inline void RTMFPSession::flush(bool echoTime,RTMFPEngine::Type type) {
	flush(0x4a,echoTime,type);
}

inline void RTMFPSession::flush(bool echoTime) {
	flush(0x4a,echoTime,prevEngineType());
}

inline bool RTMFPSession::canWriteFollowing(RTMFPWriter& writer) {
	return _pLastWriter==&writer;
}

inline bool RTMFPSession::failed() const {
	return Session::failed;
}

inline void RTMFPSession::resetWriter(RTMFPWriter& writer) {
	_flowWriters[writer.id] = &writer;
}




} // namespace Mona
