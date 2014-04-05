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

#include "Mona/RTMFP/RTMFPHandshake.h"
#include "Mona/RTMFP/RTMFProtocol.h"
#include "Mona/Util.h"
#include <openssl/evp.h>

using namespace std;



namespace Mona {

RTMFPHandshake::RTMFPHandshake(RTMFProtocol& protocol, Sessions& sessions, Invoker& invoker) : RTMFPSession(protocol, invoker, 0, RTMFP_DEFAULT_KEY, RTMFP_DEFAULT_KEY, "RTMFPHandshake"),
	_sessions(sessions),_pPeer(new Peer((Handler&)invoker)) {
	
	memcpy(_certificat,"\x01\x0A\x41\x0E",4);
	Util::Random(&_certificat[4],64);
	memcpy(&_certificat[68],"\x02\x15\x02\x02\x15\x05\x02\x15\x0E",9);
}


RTMFPHandshake::~RTMFPHandshake() {
	fail(""); // To avoid the failSignalo of RTMFPSession
	clear();
	kill(SERVER_DEATH); // true because if RTMFPHandshake is deleted it means that the sevrer is closing
}

void RTMFPHandshake::manage() {
	AttemptCounter::manage();

	// delete obsolete cookie
	auto it=_cookies.begin();
	while(it!=_cookies.end()) {
		if(it->second->obsolete()) {
			clearAttempt(it->second->tag);
			DEBUG("Obsolete cookie, ", Util::FormatHex(it->first, COOKIE_SIZE, invoker.buffer));
			delete it->second;
			_cookies.erase(it++);
		} else
			++it;
	}
}

void RTMFPHandshake::commitCookie(const UInt8* value) {
	auto it = _cookies.find(value);
	if(it==_cookies.end()) {
		WARN("RTMFPCookie ", Util::FormatHex(value, COOKIE_SIZE, invoker.buffer), " not found, maybe becoming obsolete before commiting (congestion?)");
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

void RTMFPHandshake::packetHandler(PacketReader& packet) {

	UInt8 marker = packet.read8();
	if(marker!=0x0b) {
		ERROR("Marker handshake wrong : should be 0b and not ",Format<UInt8>("%.2x",marker));
		return;
	}
	
	UInt16 time = packet.read16();
	UInt8 id = packet.read8();
	packet.shrink(packet.read16()); // length

	PacketWriter& response(this->packet());
	UInt32 oldSize(response.size());
	response.next(3); // type and size
	UInt8 idResponse = handshakeHandler(id,packet,response);
	if(idResponse>0)
		BinaryWriter(response,oldSize).write8(idResponse).write16(response.size()-oldSize-3);
	else
		response.clear(oldSize);
}
 
RTMFPSession* RTMFPHandshake::createSession(const UInt8* cookieValue) {
	map<const UInt8*,RTMFPCookie*,CompareCookies>::iterator itCookie = _cookies.find(cookieValue);
	if(itCookie==_cookies.end()) {
		WARN("Creating session for an unknown cookie '", Util::FormatHex(cookieValue, COOKIE_SIZE, invoker.buffer), "' (CPU congestion?)");
		return NULL;
	}

	RTMFPCookie& cookie(*itCookie->second);

	(UInt32&)farId = cookie.farId;

	// Create session
	RTMFPSession* pSession = &_sessions.create<RTMFPSession,Sessions::BYPEER | Sessions::BYADDRESS>(protocol<RTMFProtocol>(), invoker, farId, cookie.decryptKey(), cookie.encryptKey(),cookie.pPeer);
	(UInt32&)cookie.id = pSession->id();

	// response!
	PacketWriter& response(packet());
	response.write8(0x78);
	response.write16(cookie.length());
	cookie.read(response);
	// set the peer address
	((SocketAddress&)peer.address).set(pSession->peer.address);
	flush();
	return pSession;
}


UInt8 RTMFPHandshake::handshakeHandler(UInt8 id,PacketReader& request,PacketWriter& response) {

	switch(id){
		case 0x30: {
			
			request.next(1);
			UInt8 epdLen = request.read8()-1;

			UInt8 type = request.read8();

			string epd;
			request.readRaw(epdLen,epd);

			string tag;
			request.readRaw(16,tag);
			response.writeString8(tag);
			
			if(type == 0x0f) {

				const UInt8* peerId = (const UInt8*)epd.c_str();
				
				RTMFPSession* pSessionWanted = _sessions.find<RTMFPSession>(peerId);
	
				if(pSessionWanted) {
					if(pSessionWanted->failed())
						return 0x00; // TODO no way in RTMFP to tell "died!"
					/// Udp hole punching
					UInt32 times = attempt(tag);

					RTMFPSession* pSession(NULL);
					if(times > 0 || peer.address.host() == pSessionWanted->peer.address.host())
						pSession = _sessions.find<RTMFPSession>(peer.address);
					
					pSessionWanted->p2pHandshake(tag,peer.address,times,pSession);

					RTMFP::WriteAddress(response,pSessionWanted->peer.address, RTMFP::ADDRESS_PUBLIC);
					DEBUG("P2P address initiator exchange, ",pSessionWanted->peer.address.toString());
					for(const SocketAddress& address : pSessionWanted->peer.localAddresses) {
						RTMFP::WriteAddress(response,address, RTMFP::ADDRESS_LOCAL);
						DEBUG("P2P address initiator exchange, ",address.toString());
					}

					// add the turn address (RelayServer) if possible and required
					if (pSession && times>0) {
						if(pSession->peer.timesBeforeTurn>=times) {
							UInt16 port = invoker.relay.add(pSession->peer,pSession->peer.address,pSessionWanted->peer,pSessionWanted->peer.address);
							if(port>0) {
								Exception ex;
								SocketAddress address;
								SocketAddress::Split(pSession->peer.serverAddress, invoker.buffer);
								bool success(false);
								EXCEPTION_TO_LOG(success=address.set(ex,invoker.buffer, port),"RTMFP turn impossible")
								if (success)
									RTMFP::WriteAddress(response,address, RTMFP::ADDRESS_REDIRECTION);
							} // else ERROR already display by RelayServer class
						}
					}
					return 0x71;
				}

				DEBUG("UDP Hole punching, session ", Util::FormatHex(peerId, ID_SIZE, invoker.buffer), " wanted not found")
				set<SocketAddress> addresses;
				peer.onRendezVousUnknown(peerId,addresses);
				set<SocketAddress>::const_iterator it;
				for(it=addresses.begin();it!=addresses.end();++it) {
					if(it->host().isWildcard())
						continue;
					if(peer.address == *it)
						WARN("A client tries to connect to himself (same ",peer.address.toString()," address)");
					RTMFP::WriteAddress(response,*it,RTMFP::ADDRESS_REDIRECTION);
					DEBUG("P2P address initiator exchange, ",it->toString());
				}
				return addresses.empty() ? 0 : 0x71;
			}

			if(type == 0x0a){
				/// RTMFPHandshake
				HelloAttempt& attempt = AttemptCounter::attempt<HelloAttempt>(tag);

				Peer& peer(*_pPeer);

				// Fill peer infos
				peer.properties().clear();
				Util::UnpackUrl(epd, (string&)peer.serverAddress, (string&)peer.path,(string&)peer.query);
				Util::UnpackQuery(peer.query, peer.properties());

				Exception ex;

				set<SocketAddress> addresses;
				peer.onHandshake(attempt.count+1,addresses);
				if(!addresses.empty()) {
					set<SocketAddress>::iterator it;
					for(it=addresses.begin();it!=addresses.end();++it) {
						if (it->host().isWildcard()) {
							SocketAddress address;
							EXCEPTION_TO_LOG(address.set(ex, peer.serverAddress),"RTMFP onHandshake redirection");
							if (!ex)
								RTMFP::WriteAddress(response, address, RTMFP::ADDRESS_REDIRECTION);
						} else
							RTMFP::WriteAddress(response, *it, RTMFP::ADDRESS_REDIRECTION);
					}
					return 0x71;
				}


				// New RTMFPCookie
				RTMFPCookie* pCookie = attempt.pCookie;
				if(!pCookie) {
					((SocketAddress&)_pPeer->address).set(peer.address);
					pCookie = new RTMFPCookie(*this,invoker,tag,_pPeer);
					if (!pCookie->run(ex)) {
						delete pCookie;
						ERROR("RTMFPCookie creation, ",ex.error())
						return 0;
					}
					_pPeer.reset(new Peer((Handler&)invoker)); // reset peer
					_cookies.emplace(pCookie->value(),pCookie);
					attempt.pCookie = pCookie;
				}
				// response
				response.write8(COOKIE_SIZE);
				response.writeRaw(pCookie->value(),COOKIE_SIZE);
				// instance id (certificat in the middle)
				response.writeRaw(_certificat,sizeof(_certificat));
				return 0x70;

			} else
				ERROR("Unkown handshake first way with '", Format<UInt8>("%02x",type), "' type");
			break;
		}
		case 0x38: {
			(UInt32&)farId = request.read32();

			if(request.read7BitLongValue()!=COOKIE_SIZE) {
				ERROR("Bad handshake cookie '", Util::FormatHex(request.current(), COOKIE_SIZE, invoker.buffer), "': its size should be 64 bytes");
				return 0;
			}
	
			map<const UInt8*,RTMFPCookie*,CompareCookies>::iterator itCookie = _cookies.find(request.current());
			if(itCookie==_cookies.end()) {
				string hex;
				WARN("Unknown RTMFPCookie, certainly already connected (increase bufferSize configuration if it happens too often)");
				return 0;
			}

			RTMFPCookie& cookie(*itCookie->second);
			((SocketAddress&)cookie.pPeer->address).set(peer.address);

			if(cookie.farId==0) {
				((UInt32&)cookie.farId) = farId;
				request.next(COOKIE_SIZE);

				size_t size = (size_t)request.read7BitLongValue();
				// peerId = SHA256(farPubKey)
				EVP_Digest(request.current(),size,cookie.pPeer->id,NULL,EVP_sha256(),NULL);

				UInt32 sizeKey = request.read7BitValue()-2;
				request.next(2); // unknown
				const UInt8* initiatorKey = request.current(); request.next(sizeKey);
			
				UInt32 sizeNonce = request.read7BitValue();

				Exception ex;
				cookie.computeSecret(ex, initiatorKey,sizeKey,request.current(),sizeNonce);
				if (ex)
					ERROR("RTMFP compute secret, ", ex.error());
			} else if(cookie.id>0) {
				// Repeat cookie reponse (Session already created and keys computed)
				cookie.read(response);
				return 0x78;
			} // else Keys are computing (multi-thread)

			break;
		}
		default:
			ERROR("Unkown handshake packet id ",id);
	}

	return 0;
}




} // namespace Mona
