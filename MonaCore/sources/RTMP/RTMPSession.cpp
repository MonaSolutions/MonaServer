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

#include "Mona/RTMP/RTMPSession.h"
#include "Mona/Util.h"
#include "Mona/RTMP/RTMPSender.h"
#include "Mona/RTMP/RTMPHandshaker.h"
#include "Mona/RTMP/RTMP.h"
#include "math.h"


using namespace std;


namespace Mona {


RTMPSession::RTMPSession(const SocketAddress& address, Protocol& protocol, Invoker& invoker) : _unackBytes(0),_decrypted(0),_pThread(NULL), _chunkSize(RTMP::DEFAULT_CHUNKSIZE), _winAckSize(RTMP::DEFAULT_WIN_ACKSIZE), _handshaking(0), _pWriter(NULL), TCPSession(address, protocol, invoker) {
	dumpJustInDebug = true;
}


RTMPSession::~RTMPSession() {
	// TODO if(!died)
	// writeAMFError("Connect.AppShutdown","server is stopping");
	//_writer.close(...);

	kill();
}

void RTMPSession::kill() {
	if (died)
		return;
	_pStream.reset();
	Session::kill();
}

bool RTMPSession::buildPacket(PacketReader& packet) {

	if (_pDecryptKey && packet.available()>_decrypted) {
		RC4(_pDecryptKey.get(),packet.available()-_decrypted,packet.current()+_decrypted,(UInt8*)packet.current()+_decrypted);
		_decrypted = packet.available();
	}

	switch(_handshaking) {
		case 0:
			packet.shrink(1537);
			return packet.available() == 1537;
		case 1:
			if (packet.available() < 1536)
				return false;
			if (_decrypted>=1536)
				_decrypted -= 1536;
			packet.shrink(1536);
			return true;
	}

	if (!_pController)
		_pController.reset(new RTMPWriter(2, *this, address(),_pEncryptKey));

	dumpJustInDebug = false;

	UInt8 headerSize = packet.read8();
	UInt8 idWriter = headerSize & 0x3F;

	headerSize = 12 - (headerSize>>6)*4;
	if(headerSize==0)
		headerSize=1;

	if (packet.available() < headerSize) // want read in first the header!
		return false;

	RTMPWriter* pWriter(NULL);
	if (idWriter != 2) {
		auto it = _writers.lower_bound(idWriter);
		if (it == _writers.end() || it->first != idWriter)
			it = _writers.emplace_hint(it, piecewise_construct, forward_as_tuple(idWriter), forward_as_tuple(idWriter, (StreamSocket&)*this, peerAddress(), _pEncryptKey));
		pWriter = &it->second;
	}
	if (!pWriter)
		pWriter = _pController.get();


	RTMPChannel& channel(pWriter->channel);

	bool isRelative(true);
	if(headerSize>=4) {
		// TIME
		channel.time = packet.read24();
		if(headerSize>=8) {
			// SIZE
			channel.bodySize = packet.read24();
			// TYPE
			channel.type = (AMF::ContentType)packet.read8();
			if(headerSize>=12) {
				isRelative = false;
				// STREAM
				channel.streamId = packet.read8();
				channel.streamId += packet.read8() << 8;
				channel.streamId += packet.read8() << 16;
				channel.streamId += packet.read8() << 24;
			}
		}

		// extended timestamp
		if (channel.time >= 0xFFFFFF) {
			headerSize += 4;
			if (packet.available() < 4)
				return false;
			channel.time = packet.read32();
		}
	}

	UInt16 chunks = (UInt16)ceil(channel.bodySize / (double)_chunkSize)-1;
	UInt32 total = channel.bodySize + chunks;	

	if (packet.available()<total)
		return false;

	//// data consumed now!
	packet.shrink(total);
	total += headerSize;
	if (_decrypted>=total)
		_decrypted -= total;

	if (isRelative)
		channel.absoluteTime += channel.time;
	else
		channel.absoluteTime = channel.time;

	// remove the 0xC3 bytes
	UInt32 i = _chunkSize;
	++chunks;
	for (UInt16 chunk = 1; chunk < chunks; ++chunk) {
		UInt32 rest = channel.bodySize - i;
		memcpy((UInt8*)packet.current()+i, packet.current() + i + chunk, _chunkSize >= rest ? rest : _chunkSize);
		i += _chunkSize;
	}

	_pWriter = pWriter;
	return true;
}


void RTMPSession::packetHandler(PacketReader& packet) {
	_unackBytes += packet.position() + packet.available();

	if(_handshaking==0) {
		UInt8 handshakeType = packet.read8();
		if(handshakeType!=3 && handshakeType!=6) {
			ERROR("RTMP Handshake type unknown: ", handshakeType);
			kill();
			return;
		}
		if(!performHandshake(packet,handshakeType==6)) {
			kill();
			return;
		}
		++_handshaking;
		return;
	} else if(_handshaking==1) {
		++_handshaking;
		return;
	} else if (_handshaking == 2) {
		++_handshaking;
		// client settings
		_pController->writeProtocolSettings();
	}


	// ack if required
	if (_unackBytes >= _winAckSize) {
		_pController->writeAck(_unackBytes);
		_unackBytes = 0;
	}

	if(!_pWriter) {
		ERROR("Packet received on session ",name()," without channel indication");
        return;
    }

	// Process the packet
	RTMPChannel& channel(_pWriter->channel);

	packet.shrink(channel.bodySize);
	if (channel.type == AMF::INVOCATION_AMF3)
		packet.next(1);

	switch(channel.type) {
		case AMF::CHUNKSIZE:
			_chunkSize = packet.read32();
			break;
		case AMF::BANDWITH:
			// send a win_acksize message for accept this change
			_pController->writeWinAckSize(packet.read32());
			break;
		case AMF::WIN_ACKSIZE:
			_winAckSize = packet.read32();
			break;
		default:
			invoker.flashStream(channel.streamId, peer, _pStream).process(channel.type,channel.absoluteTime, packet,*_pWriter); // TODO peer.serverAddress?	
	}

	if (!peer.connected)
		kill();	
	_pWriter = NULL;
}

void RTMPSession::flush() {
	if (_pController)
		_pController->flush();
	if (_pStream)
		_pStream->flush();
	for (auto& it : _writers)
		it.second.flush();
}


bool RTMPSession::performHandshake(BinaryReader& packet, bool encrypted) {
	bool middle;
	const UInt8* challengeKey = RTMP::ValidateClient(packet,middle);
	shared_ptr<RTMPHandshaker> pHandshaker;
	if (challengeKey) {
		if(encrypted) {
			_pDecryptKey.reset(new RC4_KEY);
			_pEncryptKey.reset(new RC4_KEY);
		}
		packet.reset();
		pHandshaker.reset(new RTMPHandshaker(invoker.poolBuffers,peerAddress(),packet.current() + RTMP::GetDHPos(packet.current(), middle), challengeKey, middle, _pDecryptKey, _pEncryptKey));
	} else if (encrypted) {
		ERROR("Unable to validate client");
		return false;
	} else // Simple handshake!
		pHandshaker.reset(new RTMPHandshaker(invoker.poolBuffers,peerAddress(), packet.current(), packet.available()));

	Exception ex;
	_pThread = send<RTMPHandshaker>(ex, pHandshaker, _pThread);
	if (ex)
		ERROR("Handshake RTMP client failed, ",ex.error());
	return !ex;
}


} // namespace Mona
