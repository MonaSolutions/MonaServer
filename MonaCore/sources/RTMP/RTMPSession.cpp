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


using namespace std;


namespace Mona {


RTMPSession::RTMPSession(const SocketAddress& address, Protocol& protocol, Invoker& invoker) : _unackBytes(0),_decrypted(0), _chunkSize(RTMP::DEFAULT_CHUNKSIZE), _winAckSize(RTMP::DEFAULT_WIN_ACKSIZE), _handshaking(0), _pWriter(NULL), TCPSession(address, protocol, invoker) {
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

void RTMPSession::readKeys() {
	if (!_pHandshaker || _pHandshaker->failed)
		return;
	_pEncryptKey = _pHandshaker->pEncryptKey;
	_pDecryptKey = _pHandshaker->pDecryptKey;
	_pHandshaker.reset();
}

bool RTMPSession::buildPacket(PacketReader& packet) {

	if (pDecryptKey() && packet.available()>_decrypted) {
		RC4(pDecryptKey().get(),packet.available()-_decrypted,packet.current()+_decrypted,(UInt8*)packet.current()+_decrypted);
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
		_pController.reset(new RTMPWriter(2, *this, address(),_pSender,pEncryptKey()));

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
			it = _writers.emplace_hint(it, piecewise_construct, forward_as_tuple(idWriter), forward_as_tuple(idWriter, (StreamSocket&)*this, peerAddress(),_pSender, pEncryptKey()));
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
		if (_pHandshaker) // in processing, repeated packet
			return;
		_pHandshaker.reset(new RTMPHandshaker(peerAddress(), rawBuffer()));
		Exception ex;
		++_handshaking;
		send<RTMPHandshaker>(ex, _pHandshaker,NULL);
		if (ex) {
			ERROR("RTMP Handshake, ", ex.error())
			kill();
			return;
		}
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

void RTMPSession::manage() {
	if (!_pHandshaker)
		return;
	if (_pHandshaker->failed)
		kill();
}


} // namespace Mona
