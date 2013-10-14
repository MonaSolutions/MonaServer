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

#include "Mona/RTMFP/RTMFPHandshake.h"
#include "Mona/RTMFP/RTMFProtocol.h"
#include "Mona/Util.h"
#include "Poco/RandomStream.h"
#include "Poco/Format.h"
#include <openssl/evp.h>
#include <cstring>

using namespace std;
using namespace Poco;
using namespace Poco::Net;

namespace Mona {

RTMFPHandshake::RTMFPHandshake(RTMFProtocol& protocol,Gateway& gateway,Invoker& invoker) : RTMFPSession(protocol,invoker,0,Peer((Handler&)invoker),RTMFP_SYMETRIC_KEY,RTMFP_SYMETRIC_KEY,"RTMFPHandshake"),
	_gateway(gateway) {

	memcpy(_certificat,"\x01\x0A\x41\x0E",4);
	RandomInputStream().read((char*)&_certificat[4],64);
	memcpy(&_certificat[68],"\x02\x15\x02\x02\x15\x05\x02\x15\x0E",9);
}


RTMFPHandshake::~RTMFPHandshake() {
	fail(""); // To avoid the failSignal
	clear();
}

void RTMFPHandshake::manage() {
	AttemptCounter::manage();

	// delete obsolete cookie
	map<const UInt8*,RTMFPCookie*,CompareCookies>::iterator it=_cookies.begin();
	while(it!=_cookies.end()) {
		if(it->second->obsolete()) {
			clearAttempt(it->second->tag);
			DEBUG("Obsolete cookie : %s",Util::FormatHex(it->first,COOKIE_SIZE).c_str());
			delete it->second;
			_cookies.erase(it++);
		} else
			++it;
	}
}

void RTMFPHandshake::commitCookie(const UInt8* value) {
	map<const UInt8*,RTMFPCookie*,CompareCookies>::iterator it = _cookies.find(value);
	if(it==_cookies.end()) {
		WARN("RTMFPCookie %s not found, maybe becoming obsolete before commiting (congestion?)",Util::FormatHex(value,COOKIE_SIZE).c_str());
		return;
	}
	clearAttempt(it->second->tag);
	delete it->second;
	_cookies.erase(it);
	return;
}

void RTMFPHandshake::clear() {
	// delete cookies
	map<const UInt8*,RTMFPCookie*,CompareCookies>::const_iterator it;
	for(it=_cookies.begin();it!=_cookies.end();++it) {
		clearAttempt(it->second->tag);
		delete it->second;
	}
	_cookies.clear();
}

void RTMFPHandshake::createCookie(MemoryWriter& writer,HelloAttempt& attempt,const string& tag,const string& queryUrl) {
	// New RTMFPCookie
	RTMFPCookie* pCookie = attempt.pCookie;
	if(!pCookie) {
		pCookie = new RTMFPCookie(*this,invoker,tag,queryUrl);
		_cookies[pCookie->value()] =  pCookie;
		attempt.pCookie = pCookie;
	}
	writer.write8(COOKIE_SIZE);
	writer.writeRaw(pCookie->value(),COOKIE_SIZE);
}


void RTMFPHandshake::packetHandler(MemoryReader& packet) {

	UInt8 marker = packet.read8();
	if(marker!=0x0b) {
		ERROR("Marker handshake wrong : should be 0b and not %.2x",marker);
		return;
	}
	
	UInt16 time = packet.read16();
	UInt8 id = packet.read8();
	packet.shrink(packet.read16()); // length

	MemoryWriter& response(writer());
	UInt32 pos = response.position();
	response.next(3);
	UInt8 idResponse = handshakeHandler(id,packet,response);
	response.reset(pos);
	if(idResponse>0) {
		response.write8(idResponse);
		response.write16(response.length()-response.position()-2);
		flush();
	} else
		response.clear(pos);

	// reset farid to 0!
	(UInt32&)farId=0;
}

Session* RTMFPHandshake::createSession(const UInt8* cookieValue) {
	map<const UInt8*,RTMFPCookie*,CompareCookies>::iterator itCookie = _cookies.find(cookieValue);
	if(itCookie==_cookies.end()) {
		WARN("Creating session for an unknown cookie '%s' (CPU congestion?)",Util::FormatHex(cookieValue,COOKIE_SIZE).c_str());
		return NULL;
	}

	RTMFPCookie& cookie(*itCookie->second);

	// Fill peer infos
	peer.clear();
	memcpy((void*)peer.id,cookie.peerId,ID_SIZE);
	Util::UnpackUrl(cookie.queryUrl,(SocketAddress&)peer.serverAddress,(string&)peer.path,peer);
	(UInt32&)farId = cookie.farId;
	(SocketAddress&)peer.address = cookie.peerAddress;

	cookie.finalize();

	// Create session
	Session& session = _gateway.registerSession(new RTMFPSession((RTMFProtocol&)protocol,invoker,cookie.farId,peer,cookie.decryptKey,cookie.encryptKey));
	(UInt32&)cookie.id = session.id;
		
	// response!
	MemoryWriter& response(writer());
	response.write8(0x78);
	response.write16(cookie.length());
	cookie.read(response);
	flush();
	(UInt32&)farId=0; // reset farid to 0!
	return &session;
}



UInt8 RTMFPHandshake::handshakeHandler(UInt8 id,MemoryReader& request,MemoryWriter& response) {

	switch(id){
		case 0x30: {
			
			request.read8(); // passer un caractere (boite dans boite)
			UInt8 epdLen = request.read8()-1;

			UInt8 type = request.read8();

			string epd;
			request.readRaw(epdLen,epd);

			string tag;
			request.readRaw(16,tag);
			response.writeString8(tag);
			
			if(type == 0x0f) {

				const UInt8* peerId = (const UInt8*)epd.c_str();
				
				RTMFPSession* pSessionWanted = dynamic_cast<RTMFPSession*>(_gateway.session(peerId));
				if(pSessionWanted) {
					if(((Session*)pSessionWanted)->failed)
						return 0x00; // TODO no way in RTMFP to tell "died!"
					/// Udp hole punching
					UInt32 times = attempt(tag);

					RTMFPSession* pSession = (times>0 || peer.address.host()==pSessionWanted->peer.address.host()) ? dynamic_cast<RTMFPSession*>(_gateway.session(peer.address)) : NULL;
					
					if(pSession) {
						UInt16 port = invoker.relay.add(pSession->peer,pSession->peer.address,pSessionWanted->peer,pSessionWanted->peer.address);
						if(port>0) {
							response.writeAddress(SocketAddress("127.0.0.1",port),true);
							return 0x71;
						}
					}
					

					pSessionWanted->p2pHandshake(tag,peer.address,times,pSession);

					list<SocketAddress>::const_iterator it;
					for(it=pSessionWanted->peer.addresses.begin();it!=pSessionWanted->peer.addresses.end();++it) {
						response.writeAddress(*it,it==pSessionWanted->peer.addresses.begin());
						DEBUG("P2P address initiator exchange, %s",it->toString().c_str());
					}
					return 0x71;
				}

				DEBUG("UDP Hole punching : session %s wanted not found",Util::FormatHex(peerId,ID_SIZE).c_str())
				set<SocketAddress,Util::AddressComparator> addresses;
				peer.onRendezVousUnknown(peerId,addresses);
				set<SocketAddress,Util::AddressComparator>::const_iterator it;
				for(it=addresses.begin();it!=addresses.end();++it) {
					if(it->host().isWildcard())
						continue;
					if(peer.address == *it)
						WARN("A client tries to connect to himself (same %s address)",peer.address.toString().c_str());
					response.writeAddress(*it,it==addresses.begin());
					DEBUG("P2P address initiator exchange, %s",it->toString().c_str());
				}
				return addresses.empty() ? 0 : 0x71;
			}

			if(type == 0x0a){
				/// RTMFPHandshake
				HelloAttempt& attempt = AttemptCounter::attempt<HelloAttempt>(tag);

				// Fill peer infos
				peer.clear();
				Util::UnpackUrl(epd,(SocketAddress&)peer.serverAddress,(string&)peer.path,peer);
				set<SocketAddress,Util::AddressComparator> addresses;
				peer.onHandshake(attempt.count+1,addresses);
				if(!addresses.empty()) {
					set<SocketAddress,Util::AddressComparator>::iterator it;
					for(it=addresses.begin();it!=addresses.end();++it) {
						if(it->host().isWildcard())
							response.writeAddress(peer.serverAddress,it==addresses.begin());
						else
							response.writeAddress(*it,it==addresses.begin());
					}
					return 0x71;
				}

				// New RTMFPCookie
				createCookie(response,attempt,tag,epd);

				// instance id (certificat in the middle)
				response.writeRaw(_certificat,sizeof(_certificat));
				return 0x70;
			} else
				ERROR("Unkown handshake first way with '%02x' type",type);
			break;
		}
		case 0x38: {
			(UInt32&)farId = request.read32();

			if(request.read7BitLongValue()!=COOKIE_SIZE) {
				ERROR("Bad handshake cookie '%s': its size should be 64 bytes",Util::FormatHex(request.current(),COOKIE_SIZE).c_str());
				return 0;
			}
	
			map<const UInt8*,RTMFPCookie*,CompareCookies>::iterator itCookie = _cookies.find(request.current());
			if(itCookie==_cookies.end()) {
				WARN("RTMFPCookie %s unknown, maybe already connected (udpBuffer congested?)",Util::FormatHex(request.current(),COOKIE_SIZE).c_str());
				return 0;
			}

			RTMFPCookie& cookie(*itCookie->second);
			(SocketAddress&)cookie.peerAddress = peer.address;

			if(cookie.farId==0) {
				((UInt32&)cookie.farId) = farId;
				request.next(COOKIE_SIZE);

				size_t size = (size_t)request.read7BitLongValue();
				// peerId = SHA256(farPubKey)
				EVP_Digest(request.current(),size,(UInt8*)cookie.peerId,NULL,EVP_sha256(),NULL);

				UInt32 sizeKey = request.read7BitValue()-2;
				request.next(2); // unknown
				UInt8* initiatorKey = request.current(); request.next(sizeKey);
			
				UInt32 sizeNonce = request.read7BitValue();

				cookie.computeSecret(initiatorKey,sizeKey,request.current(),sizeNonce);
			} else if(cookie.id>0) {
				// Repeat cookie reponse!
				cookie.read(response);
				return 0x78;
			} // else Keys are computing (multi-thread)

			break;
		}
		default:
			ERROR("Unkown handshake packet id %u",id);
	}

	return 0;
}




} // namespace Mona
