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

RTMPWriter::RTMPWriter(UInt32 id,StreamSocket& socket,const SocketAddress& address,shared_ptr<RTMPSender>& pSender) : _address(address),id(id),_pSender(pSender), _isMain(false), _pThread(NULL), _socket(socket) {
	// TODO _qos.add
}

void RTMPWriter::writeProtocolSettings() {
	// to eliminate chunks of packet in the server->client direction
	write(AMF::CHUNKSIZE).writer.write32(0x7FFFFFFF);
	// to increase the window ack size in the server->client direction
	writeWinAckSize(2500000);
	// to increase the window ack size in the client->server direction
	AMFWriter& writer = write(AMF::BANDWITH);
	writer.writer.write32(2500000);
	writer.writer.write8(0); // hard setting
}

void RTMPWriter::flush(bool full) {
	if(state()==CONNECTING) {
		ERROR("Violation policy, impossible to flush data on a connecting writer");
		return;
	}
	if(!_pSender->available())
		return;
	_pSender->writer(_channel); // pack!
	Exception ex;
	Writer::DumpResponse(_pSender->begin(), _pSender->size(), _address);
	if (_pSender->encrypted())
		_pThread = _socket.send<RTMPSender>(ex, _pSender, _pThread);
	else
		_socket.send<RTMPSender>(ex, _pSender);
	if (ex)
		ERROR("RTMPWriter flush, ", ex.error())
	_pSender.reset(new RTMPSender(*_pSender));
}


RTMPWriter::State RTMPWriter::state(State value,bool minimal) {
	if (value==CONNECTED)
		_isMain = true;
	State state = Writer::state(value,minimal);
	if (state == CONNECTED && minimal)
		_pSender->clear();
	return state;
}

void RTMPWriter::writeRaw(const UInt8* data,UInt32 size) {
	if(size==0) {
		ERROR("Data must have at minimum the AMF type in its first byte")
		return;
	}
	MemoryReader reader(data,size);
	AMF::ContentType type((AMF::ContentType)reader.read8());
	write(type,reader.read32(),&reader);
}

// TODO essayer de comprendre toutes les routes closes!?
void RTMPWriter::close(int code) {
	if (!_isMain)
		return; // multimedia writer, not close!
	if(code>0)
		writeAMFError(code==1 ? "NetConnection.Connect.IdleTimeout" : "NetConnection.Connect.AppShutdown","Client closed by server side");
	Writer::close(code); // TODO kill if code==0???
}

AMFWriter& RTMPWriter::write(AMF::ContentType type,UInt32 time,MemoryReader* pData) {
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

	AMFWriter& writer = _pSender->writer(_channel);
	BinaryWriter& data = writer.writer;
	data.write8((headerFlag<<6)| id);

	_pSender->headerSize = 12 - 4*headerFlag;

	if (_pSender->headerSize > 0) {
		if (time<0xFFFFFF)
			data.write24(time);
		else {
			data.write24(0xFFFFFF);
			_pSender->headerSize += 4;
		}

		if (_pSender->headerSize > 4) {
			_pSender->sizePos = writer.stream.size();
			writer.stream.next(3); // For size
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
		data.writeRaw(pData->current(),pData->available());
        return AMFWriter::Null;
	}
	return writer;
}



} // namespace Mona
