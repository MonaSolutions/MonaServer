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
#include "Mona/TCPSession.h"
#include "Mona/RTSP/RTSPPacket.h"
#include "Mona/RTSP/RTSPDecoder.h"
#include "Mona/RTSP/RTSPWriter.h"
/*#include "Mona/QueryReader.h"*/


namespace Mona {

class RTSPSession : public TCPSession, public virtual Object {
public:
	RTSPSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker);
	
private:

	enum RTSPState {
		RTSP_WAITING, // All other states
		RTSP_SETTING, // A setup has been requested
		RTSP_SET,	  // Setup is done
		RTSP_PLAYING  // Already playing
	};

	void			kill(UInt32 type=NORMAL_DEATH);
	void			manage();

	UInt32			onData(PoolBuffer& pBuffer) { return _decoder.decode(pBuffer); }
	void			receive(const std::shared_ptr<RTSPPacket>& pPacket);

	void			processDescribe(const std::shared_ptr<RTSPPacket>& pPacket);
	void			processSetup(Exception& ex, const std::shared_ptr<RTSPPacket>& pPacket);
	void			processPlay(Exception& ex, const std::shared_ptr<RTSPPacket>& pPacket);

	RTSPDecoder::OnDecoded::Type	onDecoded;
	RTSPDecoder::OnDecodedEnd::Type	onDecodedEnd;

	RTSPDecoder			_decoder;
	RTSPWriter			_writer;
	Listener*			_pListener;

	RTSPState			_state;
};


} // namespace Mona
