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


RTMPSession::RTMPSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker) : _mainStream(invoker,peer),_unackBytes(0),_readBytes(0),_decrypted(0), _chunkSize(RTMP::DEFAULT_CHUNKSIZE), _winAckSize(RTMP::DEFAULT_WIN_ACKSIZE),_handshaking(0), _pWriter(NULL), TCPSession(peerAddress,file, protocol, invoker),
		onStreamStart([this](UInt16 id, FlashWriter& writer) {
			// Stream Begin signal
			(_pController ? (FlashWriter&)*_pController : writer).writeRaw().write16(0).write32(id);
		}),
		onStreamStop([this](UInt16 id, FlashWriter& writer) {
			// Stream EOF signal
			(_pController ? (FlashWriter&)*_pController : writer).writeRaw().write16(1).write32(id);
		}) {
	
	_mainStream.OnStart::subscribe(onStreamStart);
	_mainStream.OnStop::subscribe(onStreamStop);

	dumpJustInDebug = true;
}

void RTMPSession::kill(UInt32 type) {
	if (died)
		return;
	// TODO if(shutdown)
	// writeAMFError("Connect.AppShutdown","server is stopping");
	//_writer.close(...);

	_mainStream.disengage(); // disengage FlashStreams because the writers "engaged" will be deleted

	_writers.clear();
	_pWriter = NULL;
	_pController.reset();

	Session::kill(type); // at the end to "unpublish or unsubscribe" before "onDisconnection"!

	_mainStream.OnStart::unsubscribe(onStreamStart);
	_mainStream.OnStop::unsubscribe(onStreamStop);
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

		UInt8 handshakeType(*pBuffer->data());
		if(handshakeType!=3 && handshakeType!=6) {
			ERROR("RTMP Handshake type '",handshakeType,"' unknown");
			kill(PROTOCOL_DEATH);
			return size;
		}

		if(!TCPSession::receive(pBuffer))
			return size;

		_unackBytes += 1537;
		++_handshaking;

		Exception ex;
		_pHandshaker.reset(new RTMPHandshaker(handshakeType==6, peer.address, pBuffer));

		if (_pHandshaker->encrypted)
			((string&)peer.protocol) = "RTMPE";

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

	// if idWriter==0 id is encoded on the following byte, if idWriter==1 id is encoded on the both following byte
	if (idWriter < 2)
		headerSize += idWriter+1;

	if (packet.available() < headerSize) // want read in first the header!
		return false;

	if (idWriter < 2)
		idWriter = (idWriter==0 ? packet.read8() : packet.read16()) + 64;
		
	RTMPWriter* pWriter;
	if (idWriter != 2) {
		MAP_FIND_OR_EMPLACE(_writers, it, idWriter, idWriter,*this,_pSender, pEncryptKey());
		pWriter = &it->second;
	} else
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
				UInt32 streamId(packet.read8());
				streamId += packet.read8() << 8;
				streamId += packet.read8() << 16;
				streamId += packet.read8() << 24;
				if (!_mainStream.getStream(streamId, channel.pStream) && streamId) {
					ERROR("RTMPSession ",name()," indicates a non-existent ",streamId," FlashStream");
					kill(PROTOCOL_DEATH);
					return false;
				}

			}
		}
	}

	// extended timestamp (can be present for headerSize=1!)
	bool wasExtendedTime(false);
	if (channel.time >= 0xFFFFFF) {
		headerSize += 4;
		if (packet.available() < 4)
			return false;
		channel.time = packet.read32();
		wasExtendedTime = true;
	}

  //  TRACE("Writer ",pWriter->id," absolute time ",channel.absoluteTime)

	UInt32 total(channel.bodySize);
	if (!channel.pBuffer.empty()) {
		if (channel.pBuffer->size() > total) {
			ERROR("RTMPSession ",name()," with a chunked message which doesn't match the bodySize indicated");
			kill(PROTOCOL_DEATH);
			return false;
		}
		total -= channel.pBuffer->size();
	}

	if(_chunkSize && total>_chunkSize)
		total = _chunkSize;

	if (packet.available() < total)
		return false;

	//// resolve absolute time on new packet!
	if (channel.pBuffer.empty()) {
		if (isRelative)
			channel.absoluteTime += channel.time; // relative
		else
			channel.absoluteTime = channel.time; // absolute
	}
	if (wasExtendedTime) // reset channel.time
		channel.time = 0xFFFFFF;

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
		_readBytes += _unackBytes;
		DEBUG("Sending ACK : ", _readBytes, " bytes (_unackBytes: ", _unackBytes, ")")
		_pController->writeAck(_readBytes);
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

	PacketReader reader(channel.pBuffer.empty() ? packet.current() : channel.pBuffer->data(),channel.bodySize);

	switch(channel.type) {
		case AMF::ABORT:
			channel.reset(_pWriter);
			break;
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
		case AMF::ACK:
			// nothing to do, a ack message says about how many bytes have been gotten by the peer
			// RTMFP has a complexe ack mechanism and RTMP is TCP based, ack mechanism is in system layer => so useless
			break;
		default: {
			if (!channel.pStream) {
				if(_mainStream.process(channel.type,channel.absoluteTime, reader,*_pWriter) && peer.connected)
					_pWriter->isMain = true;
				else if (!died)
					kill(REJECTED_DEATH);
			} else
				channel.pStream->process(channel.type, channel.absoluteTime, reader, *_pWriter);
		}
	}

	if (_pWriter) {
		channel.pBuffer.release();
		_pWriter = NULL;
	}
}

void RTMPSession::manage() {
	if (_pHandshaker && _pHandshaker->failed)
		kill(PROTOCOL_DEATH);
	else if (peer.connected && _pController && peer.ping(30000)) // 30 sec
		_pController->writePing();
	TCPSession::manage();
}


} // namespace Mona
