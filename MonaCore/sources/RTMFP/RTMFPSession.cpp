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

#include "Mona/RTMFP/RTMFPSession.h"
#include "Mona/RTMFP/RTMFPFlow.h"
#include "Mona/RTMFP/RTMFProtocol.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"


using namespace std;


namespace Mona {

RTMFPSession::RTMFPSession(RTMFProtocol& protocol,
				Invoker& invoker,
				UInt32 farId,
				const shared_ptr<RTMFPEngine> pDecoder,
				const shared_ptr<RTMFPEngine> pEncoder,
				const shared_ptr<Peer>& pPeer) : onDecoded([this](BinaryReader& packet,const SocketAddress& address){receive(address,packet);}), onDecodedEnd([this](){flush();}), _failed(false), _pThread(NULL),farId(farId), Session(protocol, invoker, pPeer), _decoder(invoker,pDecoder), _pEncoder(pEncoder), _timesFailed(0), _timeSent(0), _nextRTMFPWriterId(0), _timesKeepalive(0), _pLastWriter(NULL), 
		onStreamStart([this](UInt16 id, FlashWriter& writer) {
			// Stream Begin signal
			writer.writeRaw().write16(0).write32(id);
		}),
		onStreamStop([this](UInt16 id, FlashWriter& writer) {
			// Stream EOF signal
			writer.writeRaw().write16(1).write32(id);
		}) {
	_pFlowNull = new RTMFPFlow(0,String::Empty,peer,invoker,*this,_pMainStream);
	_decoder.OnDecodedEnd::subscribe(onDecodedEnd);
	_decoder.OnDecoded::subscribe(onDecoded);

	// keep after _pFlowNull creation to have _pMainStream==null until here
	_pMainStream.reset(new FlashMainStream(invoker,peer));
	_pMainStream->OnStart::subscribe(onStreamStart);
	_pMainStream->OnStop::subscribe(onStreamStop);
}

RTMFPSession::RTMFPSession(RTMFProtocol& protocol,
				Invoker& invoker,
				UInt32 farId,
				const UInt8* decryptKey,
				const UInt8* encryptKey,
				const char* name) : onDecoded([this](BinaryReader& packet,const SocketAddress& address){receive(address,packet);}) , _failed(false), farId(farId), _pThread(NULL), Session(protocol, invoker, name), _decoder(invoker, decryptKey), _pEncoder(new RTMFPEngine(encryptKey, RTMFPEngine::ENCRYPT)), _timesFailed(0), _timeSent(0), _nextRTMFPWriterId(0), _timesKeepalive(0), _pLastWriter(NULL) {
	_pFlowNull = new RTMFPFlow(0,String::Empty,peer,invoker,*this,_pMainStream);
	_decoder.OnDecoded::subscribe(onDecoded);
	// No onDecodedEnd subscription for RTMFPHandshake
}

void RTMFPSession::releasePeer() {
	if (!_pMainStream)
		return;

	// Here no new sending must happen except "failSignal"
	for (auto& it : _flowWriters)
		it.second->clear();
	
	// unsubscribe peer for its groups
	peer.unsubscribeGroups();

	// delete flows
	for(auto& it : _flows)
		delete it.second;
	_flows.clear();
	
	// to remove OnStart and OnStop, and erase FlashWriters (before to erase flowWriters)
	if (_pMainStream)
		_pMainStream.reset();
	
	// delete flowWriters
	_flowWriters.clear();

	peer.onDisconnection();
}

void RTMFPSession::failSignal() {
	_failed = true;
	if(died)
		return;
	++_timesFailed;
	// no other message, just fail message, so I erase all data in first
	_pSender.reset(new RTMFPSender(invoker.poolBuffers,_pEncoder));
	_pSender->packet.write8(0x0C).write16(0);
	flush(false); // We send immediatly the fail message

	// After 6 mn we can considerated that the session is died!
	if(_timesFailed==10 || peer.lastReceptionTime.isElapsed(360000))
		kill(TIMEOUT_DEATH);
}

void RTMFPSession::kill(UInt32 type) {
	if(!_failed && type!=SOCKET_DEATH)
		failSignal();
	if(died)
		return;

	delete _pFlowNull;
	releasePeer();

	Session::kill(type);  // at the end to "unpublish or unsubscribe" before "onDisconnection"!

	// no more reception
	_decoder.OnDecoded::unsubscribe(onDecoded);
	_decoder.OnDecodedEnd::unsubscribe(onDecodedEnd);
}

void RTMFPSession::manage() {
	if(died)
		return;

	if (_failed) {
		failSignal();
		return;
	}

	// After 6 mn we considerate than the session has failed
	if(peer.lastReceptionTime.isElapsed(360000)) {
		fail("Timeout no client message");
		return;
	}

	// To accelerate the deletion of peer ghost (mainly for netgroup efficient), starts a keepalive server after 2 mn
	if(peer.lastReceptionTime.isElapsed(120000) && !keepalive()) // TODO check it!
		return;

	// Raise RTMFPWriter
	auto it=_flowWriters.begin();
	while (it != _flowWriters.end()) {
		shared_ptr<RTMFPWriter>& pWriter(it->second);
		Exception ex;
		pWriter->manage(ex, invoker);
		if (ex) {
			if (pWriter->critical) {
				fail(ex.error());
				break;
			}
			continue;
		}
		if (pWriter->consumed()) {
			_flowWriters.erase(it++);
			continue;
		}
		++it;
	}

	Session::manage();
}

bool RTMFPSession::keepalive() {
	if(!peer.connected) {
		fail("Timeout connection client");
		return false;
	}
	DEBUG("Keepalive server");
	if(_timesKeepalive==10) {
		fail("Timeout keepalive attempts");
		return false;
	}
	++_timesKeepalive;
	writeMessage(0x01,0);
	return true;
}


bool RTMFPSession::p2pHandshake(const string& tag,const SocketAddress& address,UInt32 times,Session* pSession) {
	if (_failed)
		return false;

	DEBUG("Peer newcomer address send to peer ",name()," connected");

	// the both peer see the server in a different way (and serverAddress.host()!= public address host written above),
	// Means an exterior peer, but we can't know which one is the exterior peer
	// so add an interiorAddress build with how see eachone the server on the both side
	bool hasAnExteriorPeer(pSession && pSession->peer.serverAddress.host() != peer.serverAddress.host());

	// times starts to 0
	UInt8 index=0;
	const SocketAddress* pAddress(&address);
	if(pSession && !pSession->peer.localAddresses.empty()) {
		// If two clients are on the same lan, starts with private address
		if(pSession->peer.address.host() == peer.address.host())
			++times;

		index=times%(pSession->peer.localAddresses.size()+((hasAnExteriorPeer && peer.serverAddress.host() != address.host()) ? 2 : 1));
		if (index > 0) {
			if (hasAnExteriorPeer && --index == 0)
				pAddress = NULL;
			else {
				auto it = pSession->peer.localAddresses.begin();
				advance(it, --index);
				pAddress = &(*it);
			}
		}
	}

	if (pAddress) {
		writeP2PHandshake(tag,*pAddress, index == 0 ? RTMFP::ADDRESS_PUBLIC : RTMFP::ADDRESS_LOCAL);
	} else {
		SocketAddress interiorAddress(peer.serverAddress.host(), pSession->peer.address.port());
		writeP2PHandshake(tag,interiorAddress,RTMFP::ADDRESS_PUBLIC);
	}
	flush();
	return hasAnExteriorPeer;
}

void RTMFPSession::writeP2PHandshake(const string& tag, const SocketAddress& address, RTMFP::AddressType type) {
	DEBUG("P2P address destinator exchange, ",address.toString());
	UInt16 size(0x36);
	size += (address.host().family() == IPAddress::IPv6 ? 16 : 4);
	BinaryWriter& writer = writeMessage(0x0F,size);
	writer.write8(0x22).write8(0x21).write8(0x0F).write(peer.id, ID_SIZE);
	RTMFP::WriteAddress(writer,address, type);
	writer.write(tag);
}

UInt8* RTMFPSession::packet() {
	if (!_pSender)
		_pSender.reset(new RTMFPSender(invoker.poolBuffers, _pEncoder));
	_pSender->packet.resize(RTMFP_MAX_PACKET_SIZE,false);
	return _pSender->packet.data();
}

void RTMFPSession::flush(UInt8 marker, UInt32 size) {
	if (!_pSender)
		return;
	_pSender->packet.clear(size);
	flush(false,marker);
}

void RTMFPSession::flush(bool echoTime,UInt8 marker) {
	_pLastWriter=NULL;
	if(!_pSender)
		return;
	if (!died && _pSender->available()) {
		BinaryWriter& packet(_pSender->packet);
	
		// After 30 sec, send packet without echo time
		if(peer.lastReceptionTime.isElapsed(30000))
			echoTime = false;

		if(echoTime)
			marker+=4;
		else
			packet.clip(2);

		BinaryWriter writer(packet.data()+6, 5);
		writer.write8(marker).write16(RTMFP::TimeNow());
		if (echoTime)
			writer.write16(_timeSent+RTMFP::Time(peer.lastReceptionTime.elapsed()));

		_pSender->farId = farId;
		_pSender->address.set(peer.address);

		if (packet.size() > RTMFP_MAX_PACKET_SIZE)
			ERROR("Message exceeds max RTMFP packet size on session ",name()," (",packet.size(),">",RTMFP_MAX_PACKET_SIZE,")");

		dumpResponse(packet.data() + 6, packet.size() - 6);

		Exception ex;
		_pThread = Session::send<RTMFProtocol,RTMFPSender>(ex, _pSender,_pThread);
		if (ex)
			ERROR("RTMFP flush, ", ex.error());
	}
	_pSender.reset();
}

BinaryWriter& RTMFPSession::writeMessage(UInt8 type, UInt16 length, RTMFPWriter* pWriter) {

	// No sending formated message for a failed session!
	if (_failed)
		return DataWriter::Null.packet;

	_pLastWriter=pWriter;

	UInt16 size = length + 3; // for type and size

	if(size>availableToWrite()) {
		flush(false); // send packet (and without time echo)
		
		if(size > availableToWrite()) {
			ERROR("RTMFPMessage truncated because exceeds maximum UDP packet size on session ",name());
			size = availableToWrite();
		}
		_pLastWriter=NULL;
	}

	if (!_pSender)
		_pSender.reset(new RTMFPSender(invoker.poolBuffers, _pEncoder));
	return _pSender->packet.write8(type).write16(length);
}

void RTMFPSession::receive(const SocketAddress& address, BinaryReader& packet) {

	if (!Session::receive(address, packet))
		return;

	// Read packet
	UInt8 marker = packet.read8()|0xF0;
	
	_timeSent = packet.read16();

	// with time echo
	if(marker == 0xFD) {
		UInt16 time = RTMFP::TimeNow();
		UInt16 timeEcho = packet.read16();
		if(timeEcho>time) {
			if(timeEcho-time<30)
				time=0;
			else
				time += 0xFFFF-timeEcho;
			timeEcho = 0;
		}
		peer.setPing((time-timeEcho)*RTMFP_TIMESTAMP_SCALE);
	}
	else if(marker != 0xF9)
		WARN("RTMFPPacket marker unknown : ", Format<UInt8>("%02x",marker));


	// Variables for request (0x10 and 0x11)
	UInt8 flags;
	RTMFPFlow* pFlow=NULL;
	UInt64 stage=0;
	UInt64 deltaNAck=0;

	UInt8 type = packet.available()>0 ? packet.read8() : 0xFF;

	// Can have nested queries
	while(type!=0xFF) {

		UInt16 size = packet.read16();

		PacketReader message(packet.current(),size);		

		switch(type) {
			case 0x0c :
				fail("failed on client side");
				break;

			case 0x4c :
				/// Session death!
				_failed = true; // to avoid the fail signal!!
				kill();
				return;

			/// KeepAlive
			case 0x01 :
				if(!peer.connected)
					fail("Timeout connection client");
				else
					writeMessage(0x41,0);
			case 0x41 :
				_timesKeepalive=0;
				break;

			case 0x5e : {
				// RTMFPFlow exception!
				UInt64 id = message.read7BitLongValue();
				
				RTMFPWriter* pRTMFPWriter = writer(id);
				if(pRTMFPWriter)
					pRTMFPWriter->fail("Writer rejected on session ",name());
				else
					WARN("RTMFPWriter ", id, " unfound for failed signal on session ", name());
				break;

			}
			case 0x18 :
				/// This response is sent when we answer with a Acknowledgment negative
				// It contains the id flow
				// I don't unsertand the usefulness...
				//pFlow = &flow(message.read8());
				//stage = pFlow->stageSnd();
				// For the moment, we considerate it like a exception
				fail("ack negative from server"); // send fail message immediatly
				break;

			case 0x51 : {
				/// Acknowledgment
				UInt64 id = message.read7BitLongValue();
				RTMFPWriter* pRTMFPWriter = writer(id);
				if(pRTMFPWriter)
					pRTMFPWriter->acknowledgment(message);
				else
					WARN("RTMFPWriter ",id," unfound for acknowledgment on session ",name());
				break;
			}
			/// Request
			// 0x10 normal request
			// 0x11 special request, in repeat case (following stage request)
			case 0x10 : {
				flags = message.read8();
				UInt64 idFlow = message.read7BitLongValue();
				stage = message.read7BitLongValue()-1;
				deltaNAck = message.read7BitLongValue()-1;
				
				if (_failed)
					break;

				map<UInt64,RTMFPFlow*>::const_iterator it = _flows.find(idFlow);
				pFlow = it==_flows.end() ? NULL : it->second;

				// Header part if present
				if(flags & MESSAGE_HEADER) {
					string signature;
					message.read(message.read8(),signature);

					if(!pFlow)
						pFlow = createFlow(idFlow,signature);

					if(message.read8()>0) {

						// Fullduplex header part
						if(message.read8()!=0x0A)
							WARN("Unknown fullduplex header part for the flow ",idFlow)
						else 
							message.read7BitLongValue(); // Fullduplex useless here! Because we are creating a new RTMFPFlow!

						// Useless header part 
						UInt8 length=message.read8();
						while(length>0 && message.available()) {
							WARN("Unknown message part on flow ",idFlow);
							message.next(length);
							length=message.read8();
						}
						if(length>0)
							ERROR("Bad header message part, finished before scheduled");
					}

				}
				
				if(!pFlow) {
					WARN("RTMFPFlow ",idFlow," unfound");
					if (_pFlowNull)
						((UInt64&)_pFlowNull->id) = idFlow;
					pFlow = _pFlowNull;
				}

			}	
			case 0x11 : {
				++stage;
				++deltaNAck;

				// has Header?
				if(type==0x11)
					flags = message.read8();

				// Process request
				if (pFlow && !_failed) {
					bool wasConnected(peer.connected);
					pFlow->receive(stage, deltaNAck, message, flags);
					if (!wasConnected && peer.connected) {
						for (auto& it : _flowWriters)
							it.second->open();
					}
				}
				break;
			}
			default :
				ERROR("RTMFPMessage type '", Format<UInt8>("%02x", type), "' unknown");
		}

		// Next
		packet.next(size);
		type = packet.available()>0 ? packet.read8() : 0xFF;

		// Commit RTMFPFlow (pFlow means 0x11 or 0x10 message)
		if(pFlow && type!= 0x11) {
			pFlow->commit();
			if(pFlow->consumed()) {
				if (pFlow->critical()) {
					if (!peer.connected) {
						// without connection, nothing must be sent!
						for (auto& it : _flowWriters)
							it.second->clear();
					}
					// TODO: commented because it replace other events (NetConnection.Connect.Rejected)
					// fail(); // If connection fails, log is already displayed, and so fail the whole session!
				}
				_flows.erase(pFlow->id);
				delete pFlow;
			}
			pFlow=NULL;
		}
	}
}

RTMFPWriter* RTMFPSession::writer(UInt64 id) {
	auto it = _flowWriters.find(id);
	if(it==_flowWriters.end())
		return NULL;
	return it->second.get();
}

RTMFPFlow* RTMFPSession::createFlow(UInt64 id,const string& signature) {
	if(died) {
		ERROR("Session ", name(), " is died, no more RTMFPFlow creation possible");
		return NULL;
	}
	if (!_pMainStream)
		return NULL; // has failed! use FlowNull rather

	map<UInt64,RTMFPFlow*>::iterator it = _flows.lower_bound(id);
	if(it!=_flows.end() && it->first==id) {
		WARN("RTMFPFlow ",id," has already been created on session ",name());
		return it->second;
	}

	RTMFPFlow* pFlow;

	// get flash stream process engine related by signature
	if (signature.size() > 4 && signature.compare(0, 5, "\x00\x54\x43\x04\x00", 5) == 0) // NetConnection
		pFlow = new RTMFPFlow(id, signature, peer, invoker, *this, _pMainStream);
	else if (signature.size()>3 && signature.compare(0, 4, "\x00\x54\x43\x04", 4) == 0) { // NetStream
		shared_ptr<FlashStream> pStream;
		UInt32 idSession(BinaryReader((const UInt8*)signature.c_str() + 4, signature.length() - 4).read7BitValue());
		if (!_pMainStream->getStream(idSession,pStream)) {
			ERROR("RTMFPFlow ",id," indicates a non-existent ",idSession," NetStream on session ",name());
			return NULL;
		}
		pFlow = new RTMFPFlow(id, signature, pStream, peer, invoker, *this);
	} else if(signature.size()>2 && signature.compare(0,3,"\x00\x47\x43",3)==0)  // NetGroup
		pFlow = new RTMFPFlow(id, signature, peer, invoker, *this, _pMainStream);
	else {
		ERROR("Unknown signature receive for flow creation : ", Util::FormatHex(BIN signature.data(), signature.size(), LOG_BUFFER))
		return NULL;
	}

	return _flows.emplace_hint(it, piecewise_construct, forward_as_tuple(id), forward_as_tuple(pFlow))->second;
}

void RTMFPSession::initWriter(const shared_ptr<RTMFPWriter>& pWriter) {
	while (++_nextRTMFPWriterId == 0 || !_flowWriters.emplace(_nextRTMFPWriterId, pWriter).second);
	(UInt64&)pWriter->id = _nextRTMFPWriterId;
	if (!_flows.empty())
		(UInt64&)pWriter->flowId = _flows.begin()->second->id; // newWriter will be associated to the NetConnection flow (first in _flow lists)
	if (!pWriter->signature.empty())
		DEBUG("New writer ", pWriter->id, " on session ", name());
}


shared_ptr<RTMFPWriter> RTMFPSession::changeWriter(RTMFPWriter& writer) {
	auto it = _flowWriters.find(writer.id);
	if (it == _flowWriters.end()) {
		ERROR("RTMFPWriter ", writer.id, " change impossible on session ", name())
		return shared_ptr<RTMFPWriter>(&writer);
	}
	shared_ptr<RTMFPWriter> pWriter(it->second);
	it->second.reset(&writer);
	return pWriter;
}



} // namespace Mona
