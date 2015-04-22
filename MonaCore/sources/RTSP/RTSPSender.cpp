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

#include "Mona/RTSP/RTSPSender.h"
#include "Mona/FileSystem.h"
#include "Mona/StringWriter.h"
#include "Mona/MIME.h"
#include "Mona/Logs.h"
#include "Mona/RTSP/RTSPWriter.h"
#include "Mona/HTTP/HTTP.h"

#include <fstream>

using namespace std;


namespace Mona {



RTSPSender::RTSPSender(const SocketAddress& address, RTSPPacket& request,const PoolBuffers& poolBuffers, const string& relativePath) :
	_serverAddress(request.serverAddress),
	_command(request.command),
	_address(address),
	_sizePos(0),
	_poolBuffers(poolBuffers),
	_appPath(relativePath),
	_newHeaders(false),
	_request(request),
	TCPSender("RTSPSender") {
}

void RTSPSender::onSent(Socket& socket) {
	/*Exception ex;
	socket.shutdown(ex,Socket::SEND);*/
}


bool RTSPSender::run(Exception& ex) {
	
	/// Write Content-Length if position recorded
	if (_sizePos > 0) {
		PacketWriter& packet(_pWriter->packet);
		UInt32 size = packet.size();

		// search \r\n\r\n of header part
		const UInt8* end = packet.data()+size-4;
		const UInt8* content = packet.data()+_sizePos+9;
		while (content < end) {
			if (memcmp(++content, EXPAND("\r\n\r\n")) == 0) {
				content += 4;
				break;
			}
			if (content == end)
				ERROR("HTTP header without end, invalid packet")
		}

		// write content-length
		String::Format(_buffer, end + 4 - content);
		memcpy((UInt8*)packet.data()+_sizePos,_buffer.c_str(),_buffer.size());
		packet.clear(size);
	}

	/// Dump response
	Session::DumpResponse("RTSP",data(), size(), _address);

	/// Send
	return TCPSender::run(ex);
}

DataWriter& RTSPSender::writeResponse(const char* code, bool rawWithoutLength) {
	if (_request.responseType == HTTP::CONTENT_ABSENT)
		return write(code, HTTP::CONTENT_ABSENT, NULL, NULL, 0);

	return write(code, HTTP::CONTENT_APPLICATON, "dsp", NULL, 0); // DESCRIBE response (application/dsp)
}

DataWriter& RTSPSender::write(const char* code, HTTP::ContentType type, const char* subType, const UInt8* data, UInt32 size) {
	if (_pWriter) {
		ERROR("HTTP response already written");
		return DataWriter::Null;
	}

	_newHeaders = true;

	//if (type == HTTP::CONTENT_ABSENT || data || size || !subType || !MIME::CreateDataWriter(MIME::DataType(subType), _poolBuffers, _pWriter))
		_pWriter.reset(new StringWriter(_poolBuffers));

	PacketWriter& packet = _pWriter->packet;
	packet.clear();

	// First line (HTTP/1.1 200 OK)
	UInt16 value(200);
	packet.write(EXPAND("RTSP/1.0 "));
	packet.write(code);
	if (String::ToNumber<UInt16>(code, value)) {
		packet.write(" ");
		packet.write(HTTP::CodeToMessage(value));
	}

	// Sequence number
	packet.write(EXPAND("\r\nCSeq: "));
	packet.write(String::Format(_buffer, _request.cSeq));

	// Content Type
	if (type != HTTP::CONTENT_ABSENT) {
		packet.write(EXPAND("\r\nContent-Type: "));
		packet.write(HTTP::FormatContentType(type, subType, _buffer));
		
		// Content Length
		if (data) {
			packet.write(EXPAND("\r\nContent-Length: "));
			packet.write(String::Format(_buffer, size));
		} else {
			// reserve place to add length on sending
			packet.write(EXPAND("\r\nContent-Length:           "));
			_sizePos = packet.size()-10;
		}
	}

	packet.write("\r\n\r\n");
	if (data && size > 0)
		packet.write(data, size);
	if(data)
		return DataWriter::Null;
	_pWriter->clear(packet.size());
	return *_pWriter;
}

BinaryWriter& RTSPSender::writeRaw() {
	if (_pWriter) {
		ERROR("HTTP response already written");
		return DataWriter::Null.packet;
	}
	_pWriter.reset(new StringWriter(_poolBuffers));
	return _pWriter->packet;
}

} // namespace Mona
