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
#include "Mona/FlashMainStream.h"
#include "Mona/RTMP/RTMPWriter.h"
#include "Mona/RTMP/RTMPHandshaker.h"

namespace Mona {

class RTMPSession : public TCPSession, public virtual Object {
public:

	RTMPSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker);
	
private:
	UInt32			onData(PoolBuffer& pBuffer);
	bool			buildPacket(BinaryReader& packet);
	void			receive(BinaryReader& packet);
	void			manage();
	void			flush() { Session::flush(); _mainStream.flush(); }

	void			kill(UInt32 type=NORMAL_DEATH);
	

	void							readKeys();
	const std::shared_ptr<RC4_KEY>&	pEncryptKey() { if (_handshaking == 1) readKeys(); return _pEncryptKey; }
	const std::shared_ptr<RC4_KEY>&	pDecryptKey() { if (_handshaking == 1) readKeys(); return _pDecryptKey; }

	FlashStream::OnStart::Type		onStreamStart;
	FlashStream::OnStart::Type		onStreamStop;

	UInt8							_handshaking;
	UInt32							_chunkSize;
	UInt32							_winAckSize;
	UInt32							_unackBytes;
	UInt32							_readBytes;

	std::map<UInt32,RTMPWriter>			_writers;
	std::unique_ptr<RTMPWriter>			_pController;
	RTMPWriter*							_pWriter;
	std::shared_ptr<RTMPSender>			_pSender;

	std::shared_ptr<RTMPHandshaker>		_pHandshaker;
	std::shared_ptr<RC4_KEY>			_pEncryptKey;
	std::shared_ptr<RC4_KEY>			_pDecryptKey;

	FlashMainStream						_mainStream;
	UInt32								_decrypted;
};



} // namespace Mona
