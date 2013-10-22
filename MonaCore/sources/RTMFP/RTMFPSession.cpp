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

#include "Mona/RTMFP/RTMFPSession.h"
#include "Mona/RTMFP/RTMFPCookieComputing.h"
#include "Mona/RTMFP/RTMFPDecoding.h"
#include "Mona/RTMFP/RTMFPFlow.h"
#include "Mona/RTMFP/RTMFProtocol.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"
#include "Poco/Format.h"

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {

RTMFPSession::RTMFPSession(RTMFProtocol& protocol,
				Invoker& invoker,
				UInt32 farId,
				const Peer& peer,
				const UInt8* decryptKey,
				const UInt8* encryptKey,
				const char* name) : _pThread(NULL),_pSender(new RTMFPSender(protocol)),farId(farId),Session(protocol,invoker,peer,name),_decrypt(decryptKey,RTMFPEngine::DECRYPT),_encrypt(encryptKey,RTMFPEngine::ENCRYPT),_timesFailed(0),_timeSent(0),_nextRTMFPWriterId(0),_timesKeepalive(0),_pLastWriter(NULL),_prevEngineType(RTMFPEngine::DEFAULT) {
	_pFlowNull = new RTMFPFlow(0,"",this->peer,invoker,*this);
}


RTMFPSession::~RTMFPSession() {
	if(!checked) {
		double ptr = 0;
		if (peer.getNumber("&RTMFPCookieComputing", ptr)) {
			RTMFPCookieComputing* pCookieComputing = reinterpret_cast<RTMFPCookieComputing*>(static_cast<unsigned>(ptr));
			if (pCookieComputing) {
				pCookieComputing->release();
				peer.setNumber("&RTMFPCookieComputing", 0.);
			}
		}
	}

	kill();
}


void RTMFPSession::failSignal() {
	Session::failed=true;
	if(died)
		return;
	++_timesFailed;
	MemoryWriter& writer = RTMFPSession::writer(); 
	writer.clear(11); // no other message, just fail message, so I erase all data in first
	writer.write8(0x0C);
	writer.write16(0);
	Exception ex;
	flush(ex, false); // We send immediatly the fail message
	if (ex)
		ERROR("Error during sending of the fail message : ",ex.error());

	// After 6 mn we can considerated that the session is died!
	if(_timesFailed==10 || _recvTimestamp.isElapsed(360000000))
		kill();
}

void RTMFPSession::kill() {
	if(!failed())
		failSignal();
	if(died)
		return;

	// unsubscribe peer for its groups
	peer.unsubscribeGroups();

	// delete flows
	map<UInt64,RTMFPFlow*>::const_iterator it1;
	for(it1=_flows.begin();it1!=_flows.end();++it1)
		delete it1->second;
	_flows.clear();
	delete _pFlowNull;
	
	Session::kill();
	
	// delete flowWriters
	_flowWriters.clear();
}

void RTMFPSession::manage() {
	if(died)
		return;

	Session::manage();

	if(failed()) {
		failSignal();
		return;
	}

	// After 6 mn we considerate than the session has failed
	if(_recvTimestamp.isElapsed(360000000)) {
		fail("Timeout no client message");
		return;
	}

	// To accelerate the deletion of peer ghost (mainly for netgroup efficient), starts a keepalive server after 2 mn
	if(_recvTimestamp.isElapsed(120000000) && !keepAlive())
		return;

	// Raise RTMFPWriter
	map<UInt64,AutoPtr<RTMFPWriter> >::iterator it2=_flowWriters.begin();
	while(it2!=_flowWriters.end()) {
		Exception ex;
		it2->second->manage(ex, invoker);
		if (ex) {
			if(it2->second->critical) {
				fail(ex.error());
				break;
			}
			continue;
		}
		if(it2->second->consumed()) {
			_flowWriters.erase(it2++);
			continue;
		}
		++it2;
	}

	if(!failed())
		peer.onManage();

	Exception exflush;
	flush(exflush);
	if (exflush)
		ERROR("Error during flushing rtmfp session : ", exflush.error());
}

bool RTMFPSession::keepAlive() {
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
	
	Exception ex;
	writeMessage(ex, 0x01,0);
	if (ex) {
		fail("Error of writing in keepAlive : ", ex.error());
		return false;
	}

	return true;
}


void RTMFPSession::p2pHandshake(const string& tag,const SocketAddress& address,UInt32 times,Session* pSession) {
	if(failed())
		return;

	DEBUG("Peer newcomer address send to peer ",id," connected");
	
	UInt16 size = 0x36;
	UInt8 index=0;

	const SocketAddress* pAddress = &address;
	if(pSession && peer.addresses.size()>0) {
		// If two clients are on the same lan, starts with private address
		if(pSession->peer.addresses.front().host()==peer.addresses.front().host())
			++times;

		index=times%pSession->peer.addresses.size();
		list<SocketAddress>::const_iterator it=pSession->peer.addresses.begin();
		advance(it,index);
		pAddress = &(*it);
	}
	size += (pAddress->host().family() == IPAddress::IPv6 ? 16 : 4);

	Exception ex;
	MemoryWriter& writer = writeMessage(ex, 0x0F,size);
	if (ex) {
		fail("Error trying to write p2phandshake : ", ex.error());
		return;
	}

	writer.write8(0x22);
	writer.write8(0x21);

	writer.write8(0x0F);
	writer.writeRaw(peer.id,ID_SIZE);
	
	writer.writeAddress(*pAddress,index==0);
	DEBUG("P2P address destinator exchange, ",pAddress->toString());

	writer.writeRaw(tag);

	flush(ex);
	if (ex)
		ERROR("Error during flushing in rtmfp handshake : ", ex.error());
}

MemoryReader* RTMFPSession::decode(SharedPtr<Buffer<UInt8> >& pBuffer,const SocketAddress& address) {
	RTMFPDecoding* pDecoding = new RTMFPDecoding(id,invoker,protocol,pBuffer,address);
	pDecoding->decoder = _decrypt.next(farId==0 ? RTMFPEngine::SYMMETRIC : RTMFPEngine::DEFAULT);
	_prevEngineType = pDecoding->decoder.type;
	Session::decode(pDecoding);
	return NULL;
}

void RTMFPSession::flush(Exception& ex, UInt8 marker,bool echoTime,RTMFPEngine::Type type) {
	_pLastWriter=NULL;
	if(died)
		return;

	MemoryWriter& packet(RTMFPSession::writer());
	if(packet.length()>=RTMFP_MIN_PACKET_SIZE) {

		// After 30 sec, send packet without echo time
		if(_recvTimestamp.isElapsed(30000000))
			echoTime = false;

		UInt32 offset=0;
		if(echoTime)
			marker+=4;
		else
			offset = 2;

		packet.clip(offset);
		packet.reset(6);
		packet.write8(marker);
		packet.write16(RTMFP::TimeNow());
		if(echoTime)
			packet.write16(_timeSent+RTMFP::Time(_recvTimestamp.elapsed()));
		
		_pSender->farId = farId;
		_pSender->encoder = _encrypt.next(type);
		_pSender->address = peer.address;
		_pThread = _pSender->go(ex, _pThread);
		if (ex)
			return;
		_pSender = new RTMFPSender(_pSender->handler);
	}
}

MemoryWriter& RTMFPSession::writer() {
	MemoryWriter& writer = _pSender->packet;
	if(!writer.good()) {
		if(!failed())
			WARN("Writing packet failed : the writer has certainly exceeded the size set");
		writer.reset(11);
	}
	writer.limit(RTMFP_MAX_PACKET_LENGTH);
	return writer;
}

MemoryWriter& RTMFPSession::writeMessage(Exception& ex, UInt8 type,UInt16 length,RTMFPWriter* pWriter) {

	// No sending formated message for a failed session!
	if(failed()) {
		_pSender->packet.clear(11);
		_pSender->packet.limit(11);
		return _pSender->packet;
	}

	_pLastWriter=pWriter;

	UInt16 size = length + 3; // for type and size

	if(size>_pSender->packet.available()) {
		flush(ex, false); // send packet (and without time echo)
		
		if(size > _pSender->packet.available()) {
			CRITIC("RTMFPMessage truncated because exceeds maximum UDP packet size on session ",id);
			size = _pSender->packet.available();
		}
		_pLastWriter=NULL;
	}

	MemoryWriter& writer = _pSender->packet;
	writer.limit(writer.position()+size);
	writer.write8(type);
	writer.write16(length);
	return writer;
}

void RTMFPSession::packetHandler(MemoryReader& packet) {

	_recvTimestamp.update();

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
		(UInt16&)peer.ping = (time-timeEcho)*RTMFP_TIMESTAMP_SCALE;
	}
	else if(marker != 0xF9)
		WARN("RTMFPPacket marker unknown : ", Format<UInt8>("%02x",marker));


	// Variables for request (0x10 and 0x11)
	UInt8 flags;
	RTMFPFlow* pFlow=NULL;
	UInt64 stage=0;
	UInt64 deltaNAck=0;

	UInt8 type = packet.available()>0 ? packet.read8() : 0xFF;
	bool answer = false;

	// Can have nested queries
	while(type!=0xFF) {

		UInt16 size = packet.read16();

		MemoryReader message(packet.current(),size);		

		switch(type) {
			case 0x0c :
				fail("failed on client side");
				break;

			case 0x4c :
				/// Session death!
				Session::failed=true; // to avoid the fail signal!!
				kill();
				return;

			/// KeepAlive
			case 0x01 :
				if(!peer.connected)
					fail("Timeout connection client");
				else {
					Exception ex;
					writeMessage(ex, 0x41,0);
					if (ex)
						fail("Error of writing in RTMFPSession packethandler : ", ex.error());
				}
			case 0x41 :
				_timesKeepalive=0;
				break;

			case 0x5e : {
				// RTMFPFlow exception!
				UInt64 id = message.read7BitLongValue();
				
				RTMFPWriter* pRTMFPWriter = writer(id);
				if(pRTMFPWriter)
					pRTMFPWriter->fail(format("writer rejected on session %u",this->id));
				else
					WARN("RTMFPWriter ",id," unfound for failed signal on session ",this->id);
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
					WARN("RTMFPWriter ",id," unfound for acknowledgment on session ",this->id);
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
				
				if(failed())
					break;

				map<UInt64,RTMFPFlow*>::const_iterator it = _flows.find(idFlow);
				pFlow = it==_flows.end() ? NULL : it->second;

				// Header part if present
				if(flags & MESSAGE_HEADER) {
					string signature;
					message.readString8(signature);

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
				if(pFlow)
					pFlow->fragmentHandler(stage,deltaNAck,message,flags);

				break;
			}
			default :
				ERROR("RTMFPMessage type '", Format<UInt8>("%02x", type), "' unknown");
		}

		// Next
		packet.next(size);
		type = packet.available()>0 ? packet.read8() : 0xFF;

		// Commit RTMFPFlow
		if(pFlow && type!= 0x11) {
			pFlow->commit();
			if(pFlow->consumed()) {
				_flows.erase(pFlow->id);
				delete pFlow;
			}
			pFlow=NULL;
		}
	}

	Exception ex;
	flush(ex);
	if (ex)
		ERROR("Error during flush in rtmfp packethandler : ", ex.error());
}

RTMFPWriter* RTMFPSession::writer(Mona::UInt64 id) {
	map<UInt64,AutoPtr<RTMFPWriter> >::iterator it = _flowWriters.find(id);
	if(it==_flowWriters.end())
		return NULL;
	return it->second.get();
}

RTMFPFlow& RTMFPSession::flow(Mona::UInt64 id) {
	map<UInt64,RTMFPFlow*>::const_iterator it = _flows.find(id);
	if(it==_flows.end()) {
		WARN("RTMFPFlow ",id," unfound");
		((UInt64&)_pFlowNull->id) = id;
		return *_pFlowNull;
	}
	return *it->second;
}

RTMFPFlow* RTMFPSession::createFlow(UInt64 id,const string& signature) {
	if(died) {
		ERROR("Session ",this->id," is died, no more RTMFPFlow creation possible");
		return NULL;
	}

	map<UInt64,RTMFPFlow*>::iterator it = _flows.lower_bound(id);
	if(it!=_flows.end() && it->first==id) {
		WARN("RTMFPFlow ",id," has already been created");
		return it->second;
	}
	if(it!=_flows.begin())
		--it;
	return _flows.insert(it,pair<UInt64,RTMFPFlow*>(id,new RTMFPFlow(id,signature,peer,invoker,*this)))->second;
}

void RTMFPSession::initWriter(RTMFPWriter& writer) {
	while(++_nextRTMFPWriterId==0 || _flowWriters.find(_nextRTMFPWriterId)!=_flowWriters.end());
	(UInt64&)writer.id = _nextRTMFPWriterId;
	if(_flows.begin()!=_flows.end())
		(UInt64&)writer.flowId = _flows.begin()->second->id;
	_flowWriters[_nextRTMFPWriterId] = &writer;
	if(!writer.signature.empty())
		DEBUG("New writer ",writer.id," on session ",this->id);
}



} // namespace Mona
