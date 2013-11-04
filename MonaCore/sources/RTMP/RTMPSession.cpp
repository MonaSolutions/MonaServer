/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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


RTMPSession::RTMPSession(const SocketAddress& address, Protocol& protocol, Invoker& invoker) : _mainStream(invoker, peer), _chunkSize(DEFAULT_CHUNKSIZE), _handshaking(0), _pWriter(NULL), TCPSession(address,protocol, invoker) {
}


RTMPSession::~RTMPSession() {
	// TODO if(!died)
		// writeAMFError("Connect.AppShutdown","server is stopping");
		//_writer.close(...);
	map<UInt8,RTMPWriter*>::iterator it;
	for(it=_writers.begin();it!=_writers.end();++it)
		delete it->second;
}

void RTMPSession::onNewData(const UInt8* data,UInt32 size) {
	if(_pDecryptKey)
		RC4(_pDecryptKey.get(),size,data,(UInt8*)data); // No useless to thread it (after testing)
}

bool RTMPSession::buildPacket(MemoryReader& data,UInt32& packetSize) {
	switch(_handshaking) {
		case 0:
			packetSize = 1537;
		case 1:
			packetSize = 1536;
			return true;
	}

	UInt8 headerSize = data.read8();

	UInt8 idWriter = headerSize & 0x3F;
	map<UInt8,RTMPWriter*>::iterator it = _writers.lower_bound(idWriter);
	if(it==_writers.end() || it->first!=idWriter) {
		if(it!=_writers.begin())
			--it;
		it = _writers.insert(it,pair<UInt8,RTMPWriter*>(idWriter,new RTMPWriter(idWriter,_pEncryptKey,*this)));
	}
	_pWriter = it->second;

	headerSize = 12 - (headerSize&0xC0)*4;
	if(headerSize==0)
		headerSize=1;

	RTMPChannel& channel(_pWriter->channel);

	if(headerSize>channel.headerSize)
		channel.headerSize = headerSize;
	
	UInt32 total = channel.bodySize+channel.headerSize;
	if(data.available()<total)
		return false;

	UInt32 time=0;

	if(channel.headerSize>=4) {
		// TIME
		time = data.read24();
	
		if(channel.headerSize>=8) {
			// SIZE
			channel.bodySize = data.read24();
			channel.headerSize += (UInt16)floor(double(channel.bodySize/_chunkSize)); 
			// TYPE
			channel.type = (AMF::ContentType)data.read8();
			if(channel.headerSize>=12) {
				// STREAM
				channel.streamId = data.read8();
				channel.streamId += data.read8()<<8;
				channel.streamId += data.read8()<<16;
				channel.streamId += data.read8()<<24;
			}
		}
	}

	if(time==0xFFFFFF) {
		channel.headerSize += 4;
		if(data.available()>4)
			time = data.read32();
	}
	if(channel.headerSize>=12)
		channel.time = time;
	else
		channel.time += time;

	// TODO?
	// 	if stream.typeRecv == 0x11 then headerSize = headerSize+1 end

	packetSize = channel.headerSize+channel.bodySize;
	return true;
}


void RTMPSession::packetHandler(MemoryReader& packet) {
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
	}
	
	if(!_pWriter) {
		ERROR("Packet received on session ",id," without channel indication");
		return;
	}

	// add the max chunksize possible!
	if(_writers.size()==1 && _handshaking==2) {
		++_handshaking;
		_pWriter->writeMaxChunkSize();
	}

	// Pack the packet

	RTMPChannel& channel(_pWriter->channel);

	UInt16 count = (UInt16)floor(double(channel.bodySize/_chunkSize));
	packet.shrink(packet.available()-count);
	while(count>0) {
		UInt32 nextSize = count>1 ? _chunkSize : (packet.available()-_chunkSize);
		memcpy(packet.current()+_chunkSize,packet.current()+_chunkSize+1,nextSize);
		--count;
	}

	// Process the packet
	switch(channel.type) {
		case AMF::CHUNKSIZE:
			_chunkSize = packet.read32();
			break;
		default:
			// TODO channel.streamId!
			_mainStream.process(channel.type,packet,*_pWriter); // TODO peer.serverAddress?
	}

	if(!peer.connected)
		kill();

	_pWriter->flush();

}

void RTMPSession::manage() {
	map<UInt8,RTMPWriter*>::const_iterator it;
	for(it=_writers.begin();it!=_writers.end();++it)
		it->second->flush();
}


bool RTMPSession::performHandshake(MemoryReader& packet, bool encrypted) {
	bool middle;
	const UInt8* challengeKey = RTMP::ValidateClient(packet,middle);
	shared_ptr<RTMPHandshaker> pHandshaker;
	if (challengeKey) {
		if(encrypted) {
			_pDecryptKey.reset(new RC4_KEY);
			_pEncryptKey.reset(new RC4_KEY);
		}
		packet.reset();
		pHandshaker.reset(new RTMPHandshaker(packet.current() + RTMP::GetDHPos(packet.current(), middle), challengeKey, middle, _pDecryptKey, _pEncryptKey));
	} else if (encrypted) {
		ERROR("Unable to validate client");
		return false;
	} else // Simple handshake!
		pHandshaker.reset(new RTMPHandshaker(packet.current(), packet.available()));

	Exception ex;
	send<RTMPHandshaker>(ex,pHandshaker);
	if (ex)
		ERROR("Handshake RTMP client file, ",ex.error());
	return !ex;
}


} // namespace Mona
