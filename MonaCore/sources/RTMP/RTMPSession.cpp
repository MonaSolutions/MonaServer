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
#include "math.h"


using namespace std;


namespace Mona {


RTMPSession::RTMPSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker) : _unackBytes(0),_decrypted(0), _chunkSize(RTMP::DEFAULT_CHUNKSIZE), _winAckSize(RTMP::DEFAULT_WIN_ACKSIZE),_handshaking(0), _pWriter(NULL), TCPSession(peerAddress,file, protocol, invoker),_isRelative(true) {
	dumpJustInDebug = true;
}

void RTMPSession::kill(UInt32 type) {
	if (died)
		return;
	// TODO if(shutdown)
	// writeAMFError("Connect.AppShutdown","server is stopping");
	//_writer.close(...);

	_pStream.reset();
	Session::kill(type);
}

void RTMPSession::readKeys() {
	if (!_pHandshaker || _pHandshaker->failed)
		return;
	_pEncryptKey = _pHandshaker->pEncryptKey;
	_pDecryptKey = _pHandshaker->pDecryptKey;
	_pHandshaker.reset();
}


UInt32 RTMPSession::onData(PoolBuffer& pBuffer) {

	// first hanshake
	if (!_handshaking) {
		UInt32 size(pBuffer->size());
		if (size < 1537)
			return 0;
		if (size > 1537) {
			ERROR("RTMP Handshake unknown");
			kill(PROTOCOL_DEATH);
			return size;
		}

		if(!TCPSession::receive(pBuffer))
			return size;
		_unackBytes += 1537;
		++_handshaking;

		Exception ex;
		_pHandshaker.reset(new RTMPHandshaker(peer.address, pBuffer));
		send<RTMPHandshaker>(ex, _pHandshaker, NULL); // threaded!
		if (ex) {
			ERROR("RTMP Handshake, ", ex.error())
			kill(PROTOCOL_DEATH);
		}
		return size;
	}

	const UInt8* data(pBuffer->data());
	const UInt8* end(pBuffer->data()+pBuffer->size());

	while(end-data) {
		BinaryReader packet(data, end-data);
		if (!buildPacket(packet))
			break;
		data = packet.current()+packet.available(); // next data
		receive(packet);
	}

	if (data!=pBuffer->data())
		flush();

	return data-pBuffer->data(); // consumed
}

bool RTMPSession::buildPacket(BinaryReader& packet) {

	if (pDecryptKey() && packet.available()>_decrypted) {
		RC4(pDecryptKey().get(),packet.available()-_decrypted,packet.current()+_decrypted,(UInt8*)packet.current()+_decrypted);
		_decrypted = packet.available();
	}

	if(_handshaking==1) {
		if (packet.available() < 1536)
			return false;
		if (_decrypted>=1536)
			_decrypted -= 1536;
		packet.shrink(1536);
		return true;
	}

	if (!_pController)
		_pController.reset(new RTMPWriter(2, *this,_pSender,pEncryptKey()));

	dumpJustInDebug = false;

	//Logs::Dump(packet.current(), 16);

	UInt8 headerSize = packet.read8();
	UInt32 idWriter = headerSize & 0x3F;

	headerSize = 12 - (headerSize>>6)*4;
	if(headerSize==0)
		headerSize=1;

	if (idWriter < 2)
		++headerSize;
	if (idWriter < 1)
		++headerSize;


	if (packet.available() < headerSize) // want read in first the header!
		return false;

	if (idWriter < 2) {
		idWriter = packet.read8() + 64; // second bytes + 64
		if (idWriter < 1)
			idWriter += packet.read8()*256; // third bytes*256
	}

	RTMPWriter* pWriter(NULL);
	if (idWriter != 2) {
		MAP_FIND_OR_EMPLACE(_writers, it, idWriter, idWriter,*this,_pSender, pEncryptKey());
		pWriter = &it->second;
	}
	if (!pWriter)
		pWriter = _pController.get();


	RTMPChannel& channel(pWriter->channel);
	_isRelative = true;
	if(headerSize>=4) {
		
		// TIME
		channel.time = packet.read24();
		if(headerSize>=8) {
			// SIZE
			channel.bodySize = packet.read24();
			// TYPE
			channel.type = (AMF::ContentType)packet.read8();
			if(headerSize>=12) {
				_isRelative = false;
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

  //  TRACE("Writer ",pWriter->id," absolute time ",channel.absoluteTime)

	UInt32 total(channel.bodySize);
	if (!channel.pBuffer.empty())
		total -= channel.pBuffer->size();

	if(total>_chunkSize)
		total = _chunkSize;

	if (packet.available()<total)
		return false;

	//// data consumed now!
	packet.shrink(total);
	total += headerSize;
	if (_decrypted>=total)
		_decrypted -= total;

	_pWriter = pWriter;
	return true;
}


void RTMPSession::receive(BinaryReader& packet) {

	if (!TCPSession::receive(packet))
		return;

	_unackBytes += packet.position() + packet.available();

	if (_handshaking < 2) {
		++_handshaking;
		return;
	}
	if (_handshaking == 2) {
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

	// unchunk (build)
	if (!channel.pBuffer.empty()) {
		channel.pBuffer->append(packet.current(), packet.available());
		if (channel.bodySize > channel.pBuffer->size())
			return; // wait the next piece
	} else if (channel.bodySize > packet.available()) {
		channel.pBuffer->resize(packet.available(), false);
		memcpy(channel.pBuffer->data(),packet.current(),packet.available());
		return; // wait the next piece
	}

	if (_isRelative)
		channel.absoluteTime += channel.time;
	else
		channel.absoluteTime = channel.time;

	PacketReader reader(channel.pBuffer.empty() ? packet.current() : channel.pBuffer->data(),channel.bodySize);

	if (channel.type == AMF::INVOCATION_AMF3)
		reader.next(1);

	switch(channel.type) {
		case AMF::CHUNKSIZE:
			_chunkSize = reader.read32();
			break;
		case AMF::BANDWITH:
			// send a win_acksize message for accept this change
			_pController->writeWinAckSize(reader.read32());
			break;
		case AMF::WIN_ACKSIZE:
			_winAckSize = reader.read32();
			break;
		default: {
			 // TODO peer.serverAddress?	
			if(invoker.flashStream(channel.streamId, peer, _pStream).process(channel.type,channel.absoluteTime, reader,*_pWriter) && peer.connected)
				_pWriter->isMain = true;
			else
				kill(REJECTED_DEATH);	
		}
	}

	_pWriter = NULL;
	channel.pBuffer.release();
}

void RTMPSession::manage() {
	if (_pHandshaker && _pHandshaker->failed)
		kill(PROTOCOL_DEATH);
	else if (peer.connected && _pController && peer.ping(30000)) // 30 sec
		_pController->writePing();
	TCPSession::manage();
}


} // namespace Mona
