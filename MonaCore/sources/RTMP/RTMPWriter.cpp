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

#include "Mona/RTMP/RTMPWriter.h"
#include "Mona/RTMP/RTMPSession.h"
#include "Mona/Logs.h"

using namespace std;


namespace Mona {

RTMPWriter::RTMPWriter(UInt32 id,TCPSession& session,std::shared_ptr<RTMPSender>& pSender,const shared_ptr<RC4_KEY>& pEncryptKey) : _channel(session.invoker.poolBuffers),channel(session.invoker.poolBuffers),_pSender(pSender),_pEncryptKey(pEncryptKey),id(id), isMain(false), _session(session),FlashWriter(session.peer.connected ? OPENED : OPENING,session.invoker.poolBuffers) {
	// TODO _qos.add
}

void RTMPWriter::writeProtocolSettings() {
	// to eliminate chunks of packet in the server->client direction
	write(AMF::CHUNKSIZE).packet.write32(0x7FFFFFFF);
	// to increase the window ack size in the server->client direction
	writeWinAckSize(2500000);
	// to increase the window ack size in the client->server direction
	write(AMF::BANDWITH).packet.write32(2500000).write8(0); // hard setting
	// Stream Begin - ID 0
	write(AMF::RAW).packet.write16(0).write32(0);
}

bool RTMPWriter::flush() {
	if(!_pSender || !_pSender->available())
		return false;
	Exception ex;
	if (_pEncryptKey) {
		_pSender->dump(true, _channel, _session.peer.address);
		RC4(_pEncryptKey.get(), _pSender->size(), _pSender->data(), (UInt8*)_pSender->data());
	} else
		_pSender->dump(false, _channel, _session.peer.address);
	EXCEPTION_TO_LOG(_session.send<RTMPSender>(ex, qos(), _pSender), "RTMPWriter flush")
		
	
	_pSender.reset(); // release the shared buffer (poolBuffer of AMWriter)
	return true;
}


void RTMPWriter::writeRaw(const UInt8* data,UInt32 size) {
	if(size==0) {
		ERROR("Data must have at minimum the AMF type in its first byte")
		return;
	}
	PacketReader reader(data,size);
	AMF::ContentType type((AMF::ContentType)reader.read8());
	write(type,reader.read32(),reader.current(),reader.available());
}

void RTMPWriter::close(Int32 code) {
	if (code<0)
		return;
	if (isMain) {
		if (code > 0) // TODO tester!
			writeAMFError(code == 1 ? "NetConnection.Connect.IdleTimeout" : "NetConnection.Connect.AppShutdown", "Client closed by server side");
		Writer::close();
		_session.kill();
		return;
	}
	Writer::close(code);
}

AMFWriter& RTMPWriter::write(AMF::ContentType type,UInt32 time,const UInt8* data,UInt32 size) {
	if(state()==CLOSED)
        return AMFWriter::Null;

	// KEEP it in first, to assign _channel.bodySize of the previous packet before to manipulate _channel again with the new packet
	if (!_pSender)
		_pSender.reset(new RTMPSender(_session.invoker.poolBuffers));
	AMFWriter& writer = _pSender->writer(_channel);
	BinaryWriter& packet = writer.packet;

	UInt32 absoluteTime = time;
	UInt8 headerFlag=0;
	
	// Default = Chunk Message Type 0 : full header
	if(_channel.pStream == channel.pStream) {
		if (time >= _channel.absoluteTime) {
			++headerFlag; // Chunk Message Type 1 : don't repeat the stream id
			time -= _channel.absoluteTime; // relative time!
			if (_channel.type == type && data && _channel.bodySize == size) {
				++headerFlag; // Chunk Message Type 2 : just timestamp delta
				if (_channel.time == time)
					++headerFlag; // Chunk Message Type 3 : no header
			}
		}  // else time<_channel.absoluteTime => header must be full, because can't be relative!
	} else
		_channel.pStream=channel.pStream;

	_channel.absoluteTime =	absoluteTime;
	_channel.time = time;
	_channel.type = type;
	_channel.bodySize = size;

	_pSender->headerSize = 12 - 4*headerFlag;

	if (id>319) {
		_pSender->headerSize += 2;
		packet.write8((headerFlag<<6)| 1);
		packet.write8((id-64)&0x00FF);
		packet.write8((id-64)&0xFF00);
	} else if (id > 63) {
		++_pSender->headerSize;
		packet.write8((headerFlag<<6)| 0);
		packet.write8(id-64);
	} else
		packet.write8((headerFlag<<6)| id);


	if (_pSender->headerSize > 0) {
		packet.write24(time<0xFFFFFF ? time : 0xFFFFFF);

		if (_pSender->headerSize > 4) {
			_pSender->sizePos = packet.size();
			packet.next(3); // For size
			packet.write8(type);
			if (_pSender->headerSize > 8) {
				UInt32 streamId(_channel.pStream ? _channel.pStream->id : 0);
				packet.write8(streamId);
				packet.write8(streamId >> 8);
				packet.write8(streamId >> 16);
				packet.write8(streamId >> 24);
				// if(type==AMF::DATA_AMF3) TODO?
				//	pWriter->write8(0);
			}
		}
	}
	if (time >= 0xFFFFFF) {
		packet.write32(time);
		_pSender->headerSize += 4;
	}

	if(data) {
		packet.write(data,size);
        return AMFWriter::Null;
	}
	return writer;
}


} // namespace Mona
