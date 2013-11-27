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
#include "Mona/Logs.h"

using namespace std;


namespace Mona {

RTMPWriter::RTMPWriter(UInt8 id, const shared_ptr<RC4_KEY>& pEncryptKey, StreamSocket& socket) : id(id), _pThread(NULL), _socket(socket) {
	// TODO _qos.add
	SocketAddress address;
	Exception ex; // ignore
	socket.peerAddress(ex, address);
	_pSender.reset(new RTMPSender(address,pEncryptKey));
}


void RTMPWriter::writeMaxChunkSize() {
	if(state()==CLOSED)
		return;
	BinaryWriter& writer = _pSender->writer.writer;
	writer.write8(0x42);
	writer.write32(0);
	writer.write16(4);
	writer.write8(1);
	writer.write32(0x7FFFFFFF);
}


void RTMPWriter::flush(bool full) {
	if(state()==CONNECTING) {
		ERROR("Violation policy, impossible to flush data on a connecting writer");
		return;
	}
	if(!_pSender->available())
		return;
	DumpResponse(_pSender->begin(), _pSender->size(), _socket);
	Exception ex;
	_pThread = _socket.send<RTMPSender>(ex, _pSender, _pThread);
	if (ex)
		ERROR("RTMPWriter flush, ", ex.error())
	_pSender.reset(new RTMPSender(*_pSender));
}


RTMPWriter::State RTMPWriter::state(State value,bool minimal) {
	State state = Writer::state(value,minimal);
	if(state==CONNECTED && minimal)
		_pSender->writer.clear();
	return state;
}

AMFWriter& RTMPWriter::write(AMF::ContentType type,UInt32 time,MemoryReader* pData) {
	if(state()==CLOSED)
        return AMFWriter::Null;
	return _pSender->write(id,type,time,channel.streamId,pData);
}


void RTMPWriter::writeRaw(const UInt8* data,UInt32 size) {
	if(size==0) {
		ERROR("Data must have at minimum the AMF type in its first byte")
		return;
	}
	MemoryReader reader(data,size);
	write((AMF::ContentType)reader.read8(),reader.read32(),&reader);
}


void RTMPWriter::close(int code) {
	if(code>0)
		writeAMFError(code==1 ? "NetConnection.Connect.IdleTimeout" : "NetConnection.Connect.AppShutdown","Client closed by server side");
	Writer::close(code); // TODO kill if code==0???
}


} // namespace Mona
