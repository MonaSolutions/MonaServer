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

RTMPWriter::RTMPWriter(UInt32 id,TCPSession& session,std::shared_ptr<RTMPSender>& pSender,const shared_ptr<RC4_KEY>& pEncryptKey) : _channel(session.invoker.poolBuffers),channel(session.invoker.poolBuffers),_pSender(pSender),_pEncryptKey(pEncryptKey),id(id), isMain(false), _session(session),FlashWriter(session.peer.connected ? OPENED : OPENING) {
	// TODO _qos.add
}

void RTMPWriter::writeProtocolSettings() {
	// to eliminate chunks of packet in the server->client direction
	write(AMF::CHUNKSIZE).packet.write32(0x7FFFFFFF);
	// to increase the window ack size in the server->client direction
	writeWinAckSize(2500000);
	// to increase the window ack size in the client->server direction
	write(AMF::BANDWITH).packet.write32(2500000).write8(0); // hard setting
}

void RTMPWriter::flush(bool full) {
	if(!_pSender || !_pSender->available())
		return;
	_pSender->dump(_channel,_session.peer.address);
	Exception ex;
	if (_pEncryptKey)
		RC4(_pEncryptKey.get(), _pSender->size(), _pSender->data(), (UInt8*)_pSender->data());
	EXCEPTION_TO_LOG(_session.send<RTMPSender>(ex, qos(), _pSender), "RTMPWriter flush")
		
	
	_pSender.reset(); // release the shared buffer (poolBuffer of AMWriter)
}


void RTMPWriter::writeRaw(const UInt8* data,UInt32 size) {
	if(size==0) {
		ERROR("Data must have at minimum the AMF type in its first byte")
		return;
	}
	PacketReader reader(data,size);
	AMF::ContentType type((AMF::ContentType)reader.read8());
	write(type,reader.read32(),&reader);
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

AMFWriter& RTMPWriter::write(AMF::ContentType type,UInt32 time,PacketReader* pData) {
	if(state()==CLOSED)
        return AMFWriter::Null;

	if (time < _channel.absoluteTime)
		_channel.absoluteTime = time;

	UInt32 absoluteTime = time;

	UInt8 headerFlag=0;
	if(_channel.streamId == channel.streamId) {
		++headerFlag;
		time -= _channel.absoluteTime; // relative time!
		if (_channel.type == type && pData && _channel.bodySize == pData->available()) {
			++headerFlag;
			if (_channel.time==time)
				++headerFlag;
		}
	} else
		_channel.streamId=channel.streamId;
	
	_channel.absoluteTime =	absoluteTime;
	_channel.time = time;
	_channel.type = type;

	if (!_pSender)
		_pSender.reset(new RTMPSender(_session.invoker.poolBuffers));

	AMFWriter& writer = _pSender->writer(_channel);
	BinaryWriter& data = writer.packet;

	_pSender->headerSize = 12 - 4*headerFlag;
	if (id>319) {
		_pSender->headerSize += 2;
		data.write8((headerFlag<<6)| 1);
		data.write8((id-64)&0x00FF);
		data.write8((id-64)&0xFF00);
	} else if (id > 63) {
		++_pSender->headerSize;
		data.write8((headerFlag<<6)| 0);
		data.write8(id-64);
	} else
		data.write8((headerFlag<<6)| id);


	if (_pSender->headerSize > 0) {
		if (time<0xFFFFFF)
			data.write24(time);
		else {
			data.write24(0xFFFFFF);
			_pSender->headerSize += 4;
		}

		if (_pSender->headerSize > 4) {
			_pSender->sizePos = data.size();
			data.next(3); // For size
			data.write8(type);
			if (_pSender->headerSize > 8) {
				data.write8(_channel.streamId);
				data.write8(_channel.streamId >> 8);
				data.write8(_channel.streamId >> 16);
				data.write8(_channel.streamId >> 24);
				// if(type==AMF::DATA) TODO?
				//	pWriter->write8(0);
				if (_pSender->headerSize > 12)
					data.write32(time);
			}
		}
	}

	if(pData) {
		data.write(pData->current(),pData->available());
        return AMFWriter::Null;
	}
	return writer;
}



} // namespace Mona
