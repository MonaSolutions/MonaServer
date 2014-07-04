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
#include "Mona/HTTP/HTTProtocol.h"
#include "Mona/Logs.h"

using namespace std;

namespace Mona {

HTTPWriter::HTTPWriter(TCPSession& session) : 
		_session(session),_pThread(NULL),contentType(HTTP::CONTENT_TEXT),contentSubType("html"),Writer(session.peer.connected ? OPENED : OPENING) {
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
	_lastError.assign(ex.error());
	close(code);
}

void HTTPWriter::close(Int32 code) {
	if (code < 0)
		return; // listener!
	if (code > 0 && pRequest)
		createSender(*pRequest).writeError(_lastError,code);
	else
		_session.kill(code); // kill just if no message is send
	Writer::close(code);
}

void HTTPWriter::flush(bool full) {

	if(_senders.empty())
		return;
	// TODO _qos.add(ping,_sent);
	// _sent=0;

	Exception ex;
	for (shared_ptr<HTTPSender>& pSender : _senders) {
		_pThread = _session.send<HTTPSender>(ex, qos(),pSender,_pThread);
		if (ex)
			ERROR("HTTPSender flush, ", ex.error())
	}
	_senders.clear();
}

void HTTPWriter::writeFile(const Path& file, UInt8 sortOptions, bool isApp) {

	if (!pRequest) {
		ERROR("No HTTP request to send file ",file.name())
		return;
	}
	return createSender(*pRequest).writeFile(file,sortOptions,isApp);
}	


DataWriter& HTTPWriter::write(const string& code, HTTP::ContentType type, const string& subType, const UInt8* data,UInt32 size) {
	if(state()==CLOSED)
        return DataWriter::Null;
	if (!pRequest) {
		ERROR("No HTTP request to send this reply")
		return DataWriter::Null;
	}
	return createSender(*pRequest).writer(code, type, subType, data, size);
}

DataWriter& HTTPWriter::writeResponse(UInt8 type) {
	switch (type) {
		case HTTP::RAW:
			return write("200 OK",HTTP::CONTENT_TEXT,"plain");
		case HTTP::XML:
			return write("200 OK", HTTP::CONTENT_APPLICATON,"xml");
		case HTTP::JSON:
			return write("200 OK", HTTP::CONTENT_APPLICATON,"json");
		case HTTP::SOAP:
			return write("200 OK", HTTP::CONTENT_APPLICATON,"soap+xml");
		case HTTP::CSS:
			return write("200 OK", HTTP::CONTENT_TEXT,"css");
		case HTTP::SVG:
			return write("200 OK", HTTP::CONTENT_IMAGE,"svg+xml");
	}
	return writeMessage();
}

bool HTTPWriter::writeMedia(MediaType type,UInt32 time,PacketReader& packet,Parameters& properties) {
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
			else if (pRequest->contentSubType.compare(0, 5, "x-flv")==0)
				_pMedia.reset(new FLV());
			else if(pRequest->contentSubType.compare(0,4,"mpeg")==0)
				_pMedia.reset(new MPEGTS());
			else
				ex.set(Exception::APPLICATION, "HTTP streaming for a ",pRequest->contentSubType," unsupported");
			if (ex) {
				close(ex);
				return false;
			}
			// write a HTTP header without content-length (data==NULL and size>0)
			_pMedia->write(write("200", pRequest->contentType, pRequest->contentSubType, NULL, 1).packet);
			break;
		}
		case AUDIO:
		case VIDEO: {
			if (!_pMedia || !pRequest)
				return false;
			_pMedia->write(createSender(*pRequest).writeRaw(_session.invoker.poolBuffers),type,time,packet.current(), packet.available());
			break;
		}
		default:
			return Writer::writeMedia(type,time,packet,properties);
	}
	return true;
}


} // namespace Mona
