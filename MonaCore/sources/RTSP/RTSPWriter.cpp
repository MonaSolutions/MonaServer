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

#include "Mona/RTSP/RTSPWriter.h"
#include "Mona/HTTP/HTTP.h"
#include "Mona/Logs.h"

using namespace std;

namespace Mona {

RTSPWriter::RTSPWriter(TCPSession& session) : _requestCount(0),_requesting(false),_session(session),_pThread(NULL),Writer(session.peer.connected ? OPENED : OPENING),
	audioSSRC("2783EE43"),videoSSRC("4D7C7FFD"), _portAudioRTP(0),_portAudioRTCP(0),_portVideoRTP(0),_portVideoRTCP(0),
	_sportAudioRTP(0),_sportAudioRTCP(0),_sportVideoRTP(0),_sportVideoRTCP(0) {
}

void RTSPWriter::initAudio(Exception& ex, UInt32 rtpPort,UInt32 rtcpPort, UInt32& srtpPort, UInt32& srtcpPort) {
	_portAudioRTP = rtpPort;
	_portAudioRTCP = rtcpPort;
	_sportAudioRTP = srtpPort = 6970; // TODO : get server ports dynamically (+config?)
	_sportAudioRTCP = srtcpPort = 6971;
	
	// TODO : create a unique UDPSocket for each RTSPWriter
	_pSocketAudioRTP.reset(new UDPSocket(_session.invoker.sockets));
	SocketAddress address(IPAddress::Wildcard(), srtpPort);
	if (_pSocketAudioRTP->bind(ex, address))
		NOTE("Starting RTP Audio service on port ", srtpPort)
	else
		WARN("Unable to bind address ", address.toString())

	_pSocketAudioRTCP.reset(new UDPSocket(_session.invoker.sockets));
	SocketAddress address2(IPAddress::Wildcard(), srtcpPort);
	if (_pSocketAudioRTCP->bind(ex, address2))
		NOTE("Starting RTCP Audio service on port ", srtcpPort)
	else
		WARN("Unable to bind address ", address2.toString())

	INFO("Client port used for audio : ", rtpPort, "-", rtcpPort)
}

void RTSPWriter::initVideo(Exception& ex, UInt32 rtpPort,UInt32 rtcpPort, UInt32& srtpPort, UInt32& srtcpPort) {
	_portVideoRTP = rtpPort;
	_portVideoRTCP = rtcpPort;
	_sportVideoRTP = srtpPort = 6972; // TODO : get server ports dynamically (+config?)
	_sportVideoRTCP = srtcpPort = 6973;

	// TODO : create a unique UDPSocket for each RTSPWriter
	_pSocketVideoRTP.reset(new UDPSocket(_session.invoker.sockets));
	SocketAddress address(IPAddress::Wildcard(), srtpPort);
	if (_pSocketVideoRTP->bind(ex, address))
		NOTE("Starting RTP Video service on port ", srtpPort)
	else
		WARN("Unable to bind address ", address.toString())

	_pSocketVideoRTCP.reset(new UDPSocket(_session.invoker.sockets));
	SocketAddress address2(IPAddress::Wildcard(), srtcpPort);
	if (_pSocketVideoRTCP->bind(ex, address2))
		NOTE("Starting RTP Video service on port ", srtcpPort)
	else
		WARN("Unable to bind address ", address2.toString())

	INFO("Client port used for video : ", rtpPort, "-", rtcpPort)
}

void RTSPWriter::close(const Exception& ex) {
	int code(500);
	switch(ex.code()) {
		case Exception::FILE:
			code = 404;
			break;
		case Exception::PERMISSION:
			code = 403;
			break;
		case Exception::APPLICATION:
			code = 406;
			break;
		default:
			break;
	}
	_lastError.assign(ex.error());
	close(code);
}

void RTSPWriter::close(Int32 code) {
	if (code < 0)
		return; // listener!
	RTSPSender* pSender;
	if (code > 0 && (pSender=createSender(true)))
		pSender->writeError(_lastError,code);
	else
		_session.kill(code); // kill just if no message is send
	Writer::close(code);
}

void RTSPWriter::beginRequest(const shared_ptr<RTSPPacket>& pRequest) {
	++_requestCount;
	_pRequest = pRequest;
	_pLastRequest = pRequest;
	_requesting = true;
}

void RTSPWriter::endRequest() {
	_requesting = false;
	flush();
}

RTPSender* RTSPWriter::createRTPSender(RTPSender::RtpType type, UInt16 port) {
	_RTPsenders.emplace_back(new RTPSender(_session.invoker.poolBuffers, type, _session.peer.address.host(), port));
	return &*_RTPsenders.back();
}

bool RTSPWriter::sendRTP(std::shared_ptr<RTPSender>& pSender) {
	Exception ex;
	switch(pSender->type) {
		// TODO : make it generic
		case RTPSender::RtpType::AUDIO_RTP:
			_pSocketAudioRTP->send(ex, pSender->packet.data(), pSender->packet.size(), pSender->address);
			break;
		case RTPSender::RtpType::AUDIO_RTCP:
			_pSocketAudioRTCP->send(ex, pSender->packet.data(), pSender->packet.size(), pSender->address);
			break;
		case RTPSender::RtpType::VIDEO_RTP:
			_pSocketVideoRTP->send(ex, pSender->packet.data(), pSender->packet.size(), pSender->address);
			break;
		case RTPSender::RtpType::VIDEO_RTCP:
			_pSocketVideoRTCP->send(ex, pSender->packet.data(), pSender->packet.size(), pSender->address);
			break;
	}
	if (ex)
		ERROR("RTPSender flush, ", ex.error())
	pSender.reset();
	return true;
}

RTSPSender* RTSPWriter::createSender(bool isInternResponse) {
	if(state()==CLOSED)
		return NULL;
	if (isInternResponse) {
		if (!_pRequest)
			return NULL;
		if (_pResponse)
			ERROR("RTSP Response already written")
		_pResponse.reset(new RTSPSender(_session.peer.address, *_pRequest, _session.invoker.poolBuffers, _session.peer.path));
		return &*_pResponse;
	}
	if (!_pRequest && !_pLastRequest) {
		ERROR("No RTSP request to answer")
		return NULL;
	}
	_senders.emplace_back(new RTSPSender(_session.peer.address,_pRequest ? *_pRequest : *_pLastRequest,_session.invoker.poolBuffers,_session.peer.path));
	return &*_senders.back();
}

bool RTSPWriter::send(shared_ptr<RTSPSender>& pSender) {
	Exception ex;
	if (pSender->newHeaders()) {
		if (!_requestCount)
			return false;
		if(--_requestCount==0)
			_pRequest.reset();
	}
	_pThread = _session.send<RTSPSender>(ex, qos(),pSender,_pThread);
	if (ex)
		ERROR("RTSPSender flush, ", ex.error())
	pSender.reset();
	return true;
}

bool RTSPWriter::flush() {

	if (_requesting) // during request wait the main response before flush
		return false;

	// now send just one response with header!
	if (_pResponse && !send(_pResponse))
		return false;

	// send senders
	while (!_senders.empty() && send(_senders.front()))
		_senders.pop_front();

	// send senders
	while (!_RTPsenders.empty() && sendRTP(_RTPsenders.front()))
		_RTPsenders.pop_front();
		
	return true;
}

DataWriter& RTSPWriter::writeMessage() {
	RTSPSender* pSender(createSender(false));
	if (!pSender)
		return DataWriter::Null;
	return pSender->writeResponse();
}

DataWriter& RTSPWriter::writeResponse(UInt8 type) {
	RTSPSender* pSender(createSender(true));
	if (!pSender)
		return DataWriter::Null;
	return pSender->writeResponse();
}

void RTSPWriter::writeRaw(const UInt8* data, UInt32 size) {
	RTSPSender* pSender(createSender(false));
	if (pSender)
		pSender->write("200 OK", HTTP::CONTENT_TEXT,"plain",data,size); 
}	

DataWriter& RTSPWriter::writeRaw(const char* code) {
	RTSPSender* pSender(createSender(true));
	if (pSender)
		return pSender->write(code, HTTP::CONTENT_ABSENT, NULL);
	return DataWriter::Null;
}

bool RTSPWriter::writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties) {

	switch(type) {
		case START:
		case STOP:
			break;
		case INIT: {
			// TODO changer ça par un booléen!
			if (time==DATA) {// one init by mediatype, we want here just init one time!
				if(_pSocketAudioRTP) {
					_pAudioRTP.reset(new RTP(_session.invoker.poolBuffers, "\x27\x83\xEE\x43")); // TODO : extract from audioSSRC
					
					RTPSender* pSender(createRTPSender(RTPSender::RtpType::AUDIO_RTCP,_portAudioRTCP));
					_pAudioRTP->writeRTCP(pSender->packet, MediaContainer::AUDIO, 0); // RTCP first packets
				}
				if(_pSocketVideoRTP) {
					_pVideoRTP.reset(new RTP(_session.invoker.poolBuffers, "\x4D\x7C\x7F\xFD")); // TODO : extract from videoSSRC
				
					RTPSender* pSender = createRTPSender(RTPSender::RtpType::VIDEO_RTCP,_portVideoRTCP);
					_pVideoRTP->writeRTCP(pSender->packet, MediaContainer::VIDEO, 0);
				}
			}
			break;
		}
		case AUDIO: {
			if (_pAudioRTP) {
				RTPSender* pSender(createRTPSender(RTPSender::RtpType::AUDIO_RTP,_portAudioRTP));
				_pAudioRTP->write(pSender->packet,type,time,packet.current(), packet.available());

				// TODO : make the timer intelligent and configurable
				if((time-_lastSRAudio) > 5000) {
					pSender = createRTPSender(RTPSender::RtpType::AUDIO_RTCP,_portAudioRTCP);
					_pAudioRTP->writeRTCP(pSender->packet, MediaContainer::AUDIO, time);
					_lastSRAudio = time;
				}
			}
			break;
		}
		case VIDEO: {
			if (_pVideoRTP) {
				RTPSender* pSender(createRTPSender(RTPSender::RtpType::VIDEO_RTP,_portVideoRTP));
				_pVideoRTP->write(pSender->packet,type,time,packet.current(), packet.available());

				// TODO : make the timer intelligent and configurable
				if((time-_lastSRVideo) > 5000) {
					pSender = createRTPSender(RTPSender::RtpType::VIDEO_RTCP,_portVideoRTCP);
					_pVideoRTP->writeRTCP(pSender->packet, MediaContainer::VIDEO, time);
					_lastSRVideo = time;
				}
			}
			break;
		}
		default:
			return Writer::writeMedia(type,time,packet,properties);
	}

	return true;
}


} // namespace Mona
