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

RTMFPHandshake::RTMFPHandshake(RTMFProtocol& protocol, Sessions& sessions, Invoker& invoker) : RTMFPSession(protocol, invoker, 0,RTMFP_DEFAULT_KEY ,RTMFP_DEFAULT_KEY, "RTMFPHandshake"),
	_sessions(sessions),_pPeer(new Peer((Handler&)invoker)) {
	
	((string&)_pPeer->protocol) = protocol.name;
	memcpy(_certificat,"\x01\x0A\x41\x0E",4);
	Util::Random(&_certificat[4],64);
	memcpy(&_certificat[68],"\x02\x15\x02\x02\x15\x05\x02\x15\x0E",9);

}

RTMFPHandshake::~RTMFPHandshake() {
	clear();
	kill(SOCKET_DEATH); // kill to avoid CRITIC log (nobody will kill it), SOCKET_DEATH to avoid the failSignal of RTMFPSession
}

void RTMFPHandshake::manage() {
	AttemptCounter::manage();

	// delete obsolete cookie
	auto it=_cookies.begin();
	while(it!=_cookies.end()) {
		if(it->second->obsolete()) {
			clearAttempt(it->second->tag);
			DEBUG("Obsolete cookie, ", Util::FormatHex(it->first, COOKIE_SIZE, LOG_BUFFER));
			delete it->second;
			_cookies.erase(it++);
		} else
			++it;
	}
}

void RTMFPHandshake::commitCookie(const UInt8* value) {
	auto it = _cookies.find(value);
	if(it==_cookies.end()) {
		WARN("RTMFPCookie ", Util::FormatHex(value, COOKIE_SIZE, LOG_BUFFER), " not found, maybe becoming obsolete before commiting (congestion?)");
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

void RTMFPHandshake::receive(const SocketAddress& address, BinaryReader& request) {

	if(!Session::receive(address, request))
		return;

	UInt8 marker = request.read8();
	if(marker!=0x0b) {
		ERROR("Marker handshake wrong : should be 0b and not ",Format<UInt8>("%.2x",marker));
		return;
	}

	UInt16 time = request.read16();
	UInt8 id = request.read8();
	request.shrink(request.read16()); // length

	BinaryWriter response(packet(),RTMFP_MAX_PACKET_SIZE);
	response.clear(RTMFP_HEADER_SIZE+3); // header + type and size
	UInt8 idResponse = handshakeHandler(id,address, request,response);
	if (!idResponse)
		return;
	
	BinaryWriter(response.data() + RTMFP_HEADER_SIZE, 3).write8(idResponse).write16(response.size() - RTMFP_HEADER_SIZE - 3);
	(UInt32&)farId = 0;
	flush(0x0b, response.size());
}
 
RTMFPSession* RTMFPHandshake::createSession(const UInt8* cookieValue) {
	map<const UInt8*,RTMFPCookie*,CompareCookies>::iterator itCookie = _cookies.find(cookieValue);
	if(itCookie==_cookies.end()) {
		WARN("Creating session for an unknown cookie '", Util::FormatHex(cookieValue, COOKIE_SIZE, LOG_BUFFER), "' (CPU congestion?)");
		return NULL;
	}

	RTMFPCookie& cookie(*itCookie->second);

	// Create session
	RTMFPSession* pSession = &_sessions.create<RTMFPSession,Sessions::BYPEER | Sessions::BYADDRESS>(protocol<RTMFProtocol>(), invoker, cookie.farId, cookie.decoder(), cookie.encoder(),cookie.pPeer);
	(UInt32&)cookie.id = pSession->id();

	// response!
	BinaryWriter response(packet(),RTMFP_MAX_PACKET_SIZE);
	response.clear(RTMFP_HEADER_SIZE).write8(0x78).write16(cookie.length());
	cookie.read(response);
	// set the peer address
	((SocketAddress&)peer.address).set(pSession->peer.address);
	
	(UInt32&)farId = cookie.farId;
	flush(0x0b, response.size());
	return pSession;
}


UInt8 RTMFPHandshake::handshakeHandler(UInt8 id,const SocketAddress& address, BinaryReader& request,BinaryWriter& response) {

	switch(id){
		case 0x30: {
			
			request.read7BitValue(); // = epdLen + 2 (useless)
			UInt16 epdLen = request.read7BitValue()-1;
			UInt8 type = request.read8();
			string epd;
			request.read(epdLen,epd);

			string tag;
			request.read(16,tag);
			response.write7BitValue(tag.size()).write(tag);
		
			if(type == 0x0f) {

				const UInt8* peerId((const UInt8*)epd.c_str());
				
				RTMFPSession* pSessionWanted = _sessions.findByPeer<RTMFPSession>(peerId);
	
				if(pSessionWanted) {
					if(pSessionWanted->failed())
						return 0x00; // TODO no way in RTMFP to tell "died!"
					/// Udp hole punching
					UInt32 times = attempt(tag);
		
					RTMFPSession* pSession(NULL);
					if(times > 0 || address.host() == pSessionWanted->peer.address.host()) // try in first just with public address (excepting if the both peer are on the same machine)
						pSession = _sessions.findByAddress<RTMFPSession>(address,Socket::DATAGRAM);
					
					bool hasAnExteriorPeer(pSessionWanted->p2pHandshake(tag,address,times,pSession));
					
					// public address
					RTMFP::WriteAddress(response,pSessionWanted->peer.address, RTMFP::ADDRESS_PUBLIC);
					DEBUG("P2P address initiator exchange, ",pSessionWanted->peer.address.toString());

					if (hasAnExteriorPeer && pSession->peer.serverAddress.host()!=pSessionWanted->peer.address.host()) {
						// the both peer see the server in a different way (and serverAddress.host()!= public address host written above),
						// Means an exterior peer, but we can't know which one is the exterior peer
						// so add an interiorAddress build with how see eachone the server on the both side
						SocketAddress interiorAddress(pSession->peer.serverAddress.host(), pSessionWanted->peer.address.port());
						RTMFP::WriteAddress(response,interiorAddress, RTMFP::ADDRESS_PUBLIC);
						DEBUG("P2P address initiator exchange, ",interiorAddress.toString());
					}	

					// local address
					for(const SocketAddress& address : pSessionWanted->peer.localAddresses) {
						RTMFP::WriteAddress(response,address, RTMFP::ADDRESS_LOCAL);
						DEBUG("P2P address initiator exchange, ",address.toString());
					}

					// add the turn address (RelayServer) if possible and required
					if (pSession && times>0) {
						UInt8 timesBeforeTurn(0);
						if(pSession->peer.parameters().getNumber("timesBeforeTurn",timesBeforeTurn) && timesBeforeTurn>=times) {
							UInt16 port = invoker.relayer.relay(pSession->peer.address,pSessionWanted->peer.address,20); // 20 sec de timeout is enough for RTMFP!
							if (port > 0) {
								SocketAddress address(pSession->peer.serverAddress.host(), port);
								RTMFP::WriteAddress(response, address, RTMFP::ADDRESS_REDIRECTION);
							} // else ERROR already display by RelayServer class
						}
					}
					return 0x71;
				}


				DEBUG("UDP Hole punching, session ", Util::FormatHex(peerId, ID_SIZE, LOG_BUFFER), " wanted not found")
				set<SocketAddress> addresses;
				peer.onRendezVousUnknown(peerId,addresses);
				set<SocketAddress>::const_iterator it;
				for(it=addresses.begin();it!=addresses.end();++it) {
					if(it->host().isWildcard())
						continue;
					if(address == *it)
						WARN("A client tries to connect to himself (same ", address.toString()," address)");
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
				string serverAddress;
				Util::UnpackUrl(epd, serverAddress, (string&)peer.path,(string&)peer.query);
				peer.setServerAddress(serverAddress);
				Util::UnpackQuery(peer.query, peer.properties());

				Exception ex;

				set<SocketAddress> addresses;
				peer.onHandshake(attempt.count+1,addresses);
				if(!addresses.empty()) {
					set<SocketAddress>::iterator it;
					for(it=addresses.begin();it!=addresses.end();++it) {
						if (it->host().isWildcard())
							RTMFP::WriteAddress(response, peer.serverAddress, RTMFP::ADDRESS_REDIRECTION);
						else
							RTMFP::WriteAddress(response, *it, RTMFP::ADDRESS_REDIRECTION);
					}
					return 0x71;
				}


				// New RTMFPCookie
				RTMFPCookie* pCookie = attempt.pCookie;
				if (!pCookie) {
					pCookie = new RTMFPCookie(*this, invoker, tag, _pPeer);
					if (!pCookie->run(ex)) {
						delete pCookie;
						ERROR("RTMFPCookie creation, ", ex.error())
						return 0;
					}
					_pPeer.reset(new Peer((Handler&)invoker)); // reset peer
					((string&)_pPeer->protocol) = protocol().name;
					_cookies.emplace(pCookie->value(), pCookie);
					attempt.pCookie = pCookie;
				}

				// response
				response.write8(COOKIE_SIZE);
				response.write(pCookie->value(),COOKIE_SIZE);
				// instance id (certificat in the middle)
				response.write(_certificat,sizeof(_certificat));
				return 0x70;

			} else
				ERROR("Unkown handshake first way with '", Format<UInt8>("%02x",type), "' type");
			break;
		}
		case 0x38: {
			(UInt32&)farId = request.read32();

			if(request.read7BitLongValue()!=COOKIE_SIZE) {
				ERROR("Bad handshake cookie '", Util::FormatHex(request.current(), COOKIE_SIZE, LOG_BUFFER), "': its size should be 64 bytes");
				return 0;
			}
	
			map<const UInt8*,RTMFPCookie*,CompareCookies>::iterator itCookie = _cookies.find(request.current());
			if(itCookie==_cookies.end()) {
				string hex;
				WARN("Unknown RTMFPCookie, certainly already connected (increase bufferSize configuration if it happens too often)");
				return 0;
			}

			RTMFPCookie& cookie(*itCookie->second);
			((SocketAddress&)cookie.pPeer->address).set(address);

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
