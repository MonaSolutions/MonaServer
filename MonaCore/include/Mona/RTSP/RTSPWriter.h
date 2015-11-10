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

#pragma once

#include "Mona/Mona.h"
#include "Mona/TCPSession.h"
#include "Mona/UDPSender.h"
#include "Mona/Writer.h"
#include "Mona/RTSP/RTSPSender.h"
#include "Mona/MediaContainer.h"
#include "Mona/StringWriter.h"

namespace Mona {

class RTPSender: public StringWriter {
public:
	enum RtpType {
		AUDIO_RTP,
		AUDIO_RTCP,
		VIDEO_RTP,
		VIDEO_RTCP
	};

	RTPSender(const PoolBuffers& poolBuffers, RtpType rtpType, const IPAddress& ipAddress, UInt16 port): StringWriter(poolBuffers), type(rtpType), address(ipAddress,port) {}

	const RtpType			type;
	const SocketAddress		address;
};

class RTSPWriter : public Writer, public virtual Object {
public:

	RTSPWriter(TCPSession& session);

	void			beginRequest(const std::shared_ptr<RTSPPacket>& pRequest);
	void			endRequest();

	const RTSPPacket* lastRequest() const { return _pLastRequest ? &*_pLastRequest : NULL; }

	void			abort() { _pResponse.reset(); _senders.clear(); }

	DataWriter&		writeInvocation(const char* name) { DataWriter& writer(writeMessage()); writer.writeString(name,strlen(name)); return writer; }
	DataWriter&		writeMessage();
	DataWriter&		writeResponse(UInt8 type);
	void			writeRaw(const UInt8* data, UInt32 size);
	void			close(Int32 code=0);
	void			close(const Exception& ex);

	DataWriter&     writeRaw(const char* code);

	void			initAudio(Exception& ex, UInt32 rtpPort, UInt32 rtcpPort, UInt32& srtpPort, UInt32& srtcpPort);
	void			initVideo(Exception& ex, UInt32 rtpPort, UInt32 rtcpPort, UInt32& srtpPort, UInt32& srtcpPort);
	
	std::string		audioSSRC;
	std::string		videoSSRC;
private:
	bool			flush();

	bool			writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties);

	RTPSender*		createRTPSender(RTPSender::RtpType type, UInt16 port);
	bool			sendRTP(std::shared_ptr<RTPSender>& pSender);
	RTSPSender*     createSender(bool isInternResponse);
	bool			send(std::shared_ptr<RTSPSender>& pSender);

	std::unique_ptr<RTP>						_pAudioRTP;
	std::unique_ptr<RTP>						_pVideoRTP;
	TCPSession&									_session;
	PoolThread*									_pThread;
	std::shared_ptr<RTSPSender>					_pResponse;
	std::deque<std::shared_ptr<RTSPSender>>		_senders;
	bool										_isMain;
	std::string									_lastError;
	std::shared_ptr<RTSPPacket>					_pRequest;
	UInt32										_requestCount;
	std::shared_ptr<RTSPPacket>					_pLastRequest;
	bool										_requesting;

	
	std::deque<std::shared_ptr<RTPSender>>		_RTPsenders;

	
	std::shared_ptr<UDPSocket>					_pSocketAudioRTP;
	std::shared_ptr<UDPSocket>					_pSocketAudioRTCP;
	std::shared_ptr<UDPSocket>					_pSocketVideoRTP;
	std::shared_ptr<UDPSocket>					_pSocketVideoRTCP;

	UInt16										_portAudioRTP;
	UInt16										_portAudioRTCP;
	UInt16										_portVideoRTP;
	UInt16										_portVideoRTCP;

	UInt16										_sportAudioRTP;
	UInt16										_sportAudioRTCP;
	UInt16										_sportVideoRTP;
	UInt16										_sportVideoRTCP;

	UInt32										_lastSRAudio;	/// last Audio Sender Report (RTCP)
	UInt32										_lastSRVideo;	/// last Video SR
};



} // namespace Mona
