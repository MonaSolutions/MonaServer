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

#include "Mona/HTTP/HTTPWriter.h"
#include "Mona/HTTP/HTTP.h"

using namespace std;

namespace Mona {

HTTPWriter::HTTPWriter(TCPClient& socket) : _socket(socket),_pThread(NULL),_initMedia(false) {
	
}

void HTTPWriter::close(const Exception& ex) {
	int code(500);
	switch(ex.code()) {
		case Exception::FILE:
			code = 404;
			break;
		case Exception::PERMISSION:
			code = 403;
			break;
		case Exception::APPLICATION:
			code = 503;
			break;
	}
	_buffer.assign(ex.error());
	close(code);
}

void HTTPWriter::close(int code) {
	if (code >= 0) {
		if (code > 0)
			createSender().writeError(code,_buffer,true);
		_socket.disconnect();
	}
	Writer::close(code);
}

void HTTPWriter::flush(bool full) {
	if(state()==CONNECTING) {
		ERROR("Violation policy, impossible to flush data on a connecting writer");
		return;
	}

	if(_senders.empty())
		return;
	// TODO _qos.add(ping,_sent);
	// _sent=0;
	
	timeout.update();

	Exception ex;
	for (shared_ptr<HTTPSender>& pSender : _senders) {
		_pThread = _socket.send<HTTPSender>(ex, pSender,_pThread);
		if (ex)
			ERROR("HTTPSender flush, ", ex.error())
		else
			_sent.emplace_back(pSender);
	}
	_senders.clear();
}


HTTPWriter::State HTTPWriter::state(State value,bool minimal) {
	State state = Writer::state(value,minimal);
	if(state==CONNECTED && minimal)
		_senders.clear();
	return state;
}

DataWriter& HTTPWriter::write(const string& code, HTTP::ContentType type, const string& subType, const UInt8* data,UInt32 size) {
	if(state()==CLOSED)
        return DataWriter::Null;
	return createSender().writer(code, type, subType, data, size);
}

DataWriter& HTTPWriter::writeResponse(UInt8 type) {
	switch (type) {
		case RAW:
			return write("200 OK",HTTP::CONTENT_TEXT,"plain; charset=utf-8");
		case XML:
			return write("200 OK", HTTP::CONTENT_APPLICATON,"xml; charset=utf-8");
		case JSON:
			return write("200 OK", HTTP::CONTENT_APPLICATON,"json; charset=utf-8");
		case SOAP:
			return write("200 OK", HTTP::CONTENT_APPLICATON,"soap+xml; charset=utf-8");
		case CSS:
			return write("200 OK", HTTP::CONTENT_TEXT,"css; charset=utf-8");
		case SVG:
			return write("200 OK", HTTP::CONTENT_IMAGE,"svg+xml; charset=utf-8");
	}
	return write("200 OK");
}

bool HTTPWriter::writeMedia(MediaType type,UInt32 time,MemoryReader& data) {
	if(state()==CLOSED)
		return true;
	switch(type) {
		case START:
		case STOP:
			break;
		case INIT: {
			if (time>0) // one init by mediatype, we want here just init one time!
				break;
			Exception ex;
			if (!pRequest)
				ex.set(Exception::APPLICATION, "HTTP streaming without request related");
			else if(pRequest->contentSubType == "x-flv")
				_mediaType = MediaContainer::FLV;
			else if(pRequest->contentSubType == "mpeg")
				_mediaType = MediaContainer::MPEG_TS;
			else
				ex.set(Exception::APPLICATION, "HTTP streaming for a ",pRequest->contentSubType," unsupported");
			if (ex) {
				close(ex);
				break;
			}
			// write a HTTP header without content-length (data==NULL and size>0)
			write("200", pRequest->contentType, pRequest->contentSubType, NULL, 1);
			_initMedia = true;
			break;
		}
		case AUDIO:
		case VIDEO: {
			BinaryWriter& writer = createSender().writeRaw();
			if (_initMedia) {
				// write header the first time
				MediaContainer::Write(_mediaType,writer);
				_initMedia = false;
			}
			MediaContainer::Write(_mediaType,writer,type,time,data.current(), data.available());
			break;
		}
		default:
			return Writer::writeMedia(type,time,data);
	}
	return true;
}


} // namespace Mona
