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

#include "Mona/RTSP/RTSPSession.h"
#include "Mona/QueryReader.h"
#include "Mona/HTTP/HTTP.h"
#if defined(_WIN32)
#define sscanf sscanf_s
#endif


using namespace std;


namespace Mona {


RTSPSession::RTSPSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker) :
	TCPSession(peerAddress, file,protocol, invoker), _writer(*this), _pListener(NULL),
	_decoder(invoker),_state(RTSP_WAITING),
	onDecoded([this](const std::shared_ptr<RTSPPacket>& pPacket, const SocketAddress& address) {receive(pPacket); }), 
	onDecodedEnd([this]() {flush(); }) {

	_decoder.OnDecodedEnd::subscribe(onDecodedEnd);
	_decoder.OnDecoded::subscribe(onDecoded);
}

void RTSPSession::kill(UInt32 type){
	if(died)
		return;

	if (_pListener) {
		invoker.unsubscribe(peer, _pListener->publication.name());
		_pListener = NULL;
	}
	TCPSession::kill(type);

	// no more reception
	_decoder.OnDecoded::unsubscribe(onDecoded);
	_decoder.OnDecodedEnd::unsubscribe(onDecodedEnd);
}

void RTSPSession::receive(const shared_ptr<RTSPPacket>& pPacket) {

	if (!TCPSession::receive(*pPacket))
		return;

	// invalid packet?
	
	if (pPacket->exception)
		return _writer.close(pPacket->exception);

	_writer.beginRequest(pPacket);

	//// Disconnection if path has changed
	/*if(peer.connected && (pPacket->connection&HTTP::CONNECTION_UPDATE || String::ICompare(peer.path,pPacket->path)!=0 || String::ICompare(peer.query,pPacket->query)!=0)) // if path change or query!! (onConnection grab query!)
		peer.onDisconnection();*/

	////  Fill peers infos
	peer.setPath(pPacket->path);
	peer.setQuery(pPacket->query);
	peer.setServerAddress(pPacket->serverAddress);
	peer.properties().clear();
	peer.properties().setNumber("version", pPacket->version);
	for (auto it : pPacket->headers)
		peer.properties().setString(it.first, it.second);

	// Create parameters for onConnection or a GET onRead/onWrite/onMessage
	PacketReader query(BIN peer.query.data(), peer.query.size());
	QueryReader parameters(query);
		
	// onConnection
	Exception ex;
	if (!peer.connected)
		peer.onConnection(ex, _writer, parameters);

	// Prepare response
	if (!ex && peer.connected) {
		switch(pPacket->command) {
			case RTSP::COMMAND_GET_PARAMETER:
			case RTSP::COMMAND_OPTIONS:
				HTTP_BEGIN_HEADER(_writer.writeRaw("200 OK").packet)
					HTTP_ADD_HEADER("Public", EXPAND("DESCRIBE, SETUP, TEARDOWN, PLAY, GET_PARAMETER, SET_PARAMETER"))
				HTTP_END_HEADER
				break;
			case RTSP::COMMAND_DESCRIBE:
				processDescribe(pPacket);
				break;
			case RTSP::COMMAND_SETUP:
				processSetup(ex, pPacket);
				break;
			case RTSP::COMMAND_PLAY:
				processPlay(ex, pPacket);
				break;
			default:
				WARN("Unhandled command type : ", pPacket->command)
				break;
		}
	}

	// Any error?
	if (ex)
		_writer.close(ex);
	else if(!peer.connected)
		kill(REJECTED_DEATH);

	_writer.endRequest();
}

void RTSPSession::manage() {
	// do not kill session if playing
	if(_state!=RTSP_PLAYING)
		TCPSession::manage();
}

void RTSPSession::processDescribe(const shared_ptr<RTSPPacket>& pPacket) {

	if(pPacket->headers["Accept"] && !strstr(pPacket->headers["Accept"],"application/sdp"))
		_writer.close(501); // Not Implemented : Only application/sdp is supported
	else {
		// write headers
		string buffId;
		BinaryWriter& writer(_writer.writeResponse(200).packet);
		HTTP_BEGIN_HEADER(writer)
			HTTP_ADD_HEADER("Content-Base", pPacket->url)
			HTTP_ADD_HEADER("Session", String::Format(buffId, id(), ";timeout=", invoker.getNumber<UInt16>("RTSP.timeout")))
		HTTP_END_HEADER

		// Write SDP content
		writer.write("v=0\r\n"); // version
		writer.write("o=- ").write(String::Format(buffId, id())).write(' ').write(buffId).write(" IN ").write(peer.address.family() == IPAddress::IPv4? "IP4 " : "IP6 ")
			.write(peer.address.host().toString()).write("\r\n"); // origin and session id
		writer.write("s=").write(pPacket->file.name()).write("\r\n"); // session name
		writer.write("t=0 0\r\n"); // active session time (permanent)

		// TODO : subscribe here and get media parameters dinamycally

		 // media audio
		writer.write("m=audio 0 RTP/AVP 14\r\n"); // description
		writer.write("a=rtpmap:14 MPA/22050/1\r\n"); // description
		writer.write("a=control:trackID=1\r\n"); // id
		/*writer.write("m=audio 0 RTP/AVP 98\r\n"); // description
		writer.write("a=rtpmap:98 speex/8000\r\n"); // description
		*/
		 // media video
		writer.write("m=video 0 RTP/AVP 97\r\n");
		writer.write("a=rtpmap:97 H264/90000\r\n"); // description
		writer.write("a=control:trackID=2\r\n");
	}
}

void RTSPSession::processSetup(Exception& ex, const shared_ptr<RTSPPacket>& pPacket) {

	const char* clientPort = NULL;
	const char* test = pPacket->headers["Transport"];
	if (!pPacket->trackID) {
		ex.set(Exception::APPLICATION, "SETUP request without trackID");
	} else if(!pPacket->headers["Transport"] || !(clientPort = strstr(pPacket->headers["Transport"],"client_port="))) {
		ex.set(Exception::APPLICATION, "SETUP request without Transport or client_port");
	} else {
		UInt32 rtpPort = 0,rtcpPort = 0;
		UInt32 srtpPort = 0,srtcpPort = 0;
		sscanf(clientPort, "client_port=%d-%d", &rtpPort, &rtcpPort);
		if(pPacket->trackID==1)
			_writer.initAudio(ex, rtpPort, rtcpPort, srtpPort, srtcpPort);
		else
			_writer.initVideo(ex, rtpPort, rtcpPort, srtpPort, srtcpPort);

		string buffer;
		HTTP_BEGIN_HEADER(_writer.writeResponse(200).packet)
			String::Format(buffer, "RTP/AVP;unicast;client_port=", rtpPort, '-', rtcpPort, 
				";server_port=", srtpPort, '-', srtcpPort, ";ssrc=", (pPacket->trackID == 1)? _writer.audioSSRC : _writer.videoSSRC);
			HTTP_ADD_HEADER("Transport", buffer)
			HTTP_ADD_HEADER("Session", String::Format(buffer, id(), ";timeout=", invoker.getNumber<UInt16>("RTSP.timeout")))
		HTTP_END_HEADER
		
		_state = RTSP_SET;
	}
}

void RTSPSession::processPlay(Exception& ex, const std::shared_ptr<RTSPPacket>& pPacket) {

	if(_state != RTSP_SET)
		ex.set(Exception::APPLICATION, "Unable to play the stream in the current state (", _state, ")");
	else if(_pListener = invoker.subscribe(ex,peer,pPacket->file.baseName(),_writer)) {

		_state = RTSP_PLAYING;
		string buffer;
		HTTP_BEGIN_HEADER(_writer.writeResponse(200).packet)
			HTTP_ADD_HEADER("RTP-Info", pPacket->url)
			HTTP_ADD_HEADER("Session", String::Format(buffer, id(), ";timeout=", invoker.getNumber<UInt16>("RTSP.timeout")))
		HTTP_END_HEADER
	}
}

} // namespace Mona
