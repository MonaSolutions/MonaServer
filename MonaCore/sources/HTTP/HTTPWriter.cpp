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

HTTPWriter::HTTPWriter(TCPSession& session) : _session(session),_pThread(NULL),Writer(session.peer.connected ? OPENED : OPENING) {
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
	HTTPSender* pSender;
	if (code > 0 && (pSender=createSender()))
		pSender->writeError(_lastError,code);
	else
		_session.kill(code); // kill just if no message is send
	Writer::close(code);
}

HTTPSender* HTTPWriter::createSender() {
	if (_pRequest) {
		_pSender.reset(new HTTPSender(_session.peer.address,*_pRequest,_session.invoker.poolBuffers,_session.peer.path));
		_pRequest.reset();
		return &*_pSender;
	}
	if (_pFirstRequest) {
		_pushSenders.emplace_back(new HTTPSender(_session.peer.address,*_pFirstRequest,_session.invoker.poolBuffers,_session.peer.path));
		return &*_pushSenders.back();
	}
	return NULL;
}

void HTTPWriter::flush(bool withPush) {
	Exception ex;
	if (_pSender) {
		_pThread = _session.send<HTTPSender>(ex, qos(),_pSender,_pThread);
		if (ex)
			ERROR("HTTPSender flush, ", ex.error())
		_pSender.reset();
	} else {
		bool one(false);
		while (!_pushSenders.empty()) {
			const shared_ptr<HTTPSender>& pSender(_pushSenders.front());
			if (one && pSender->written())
				break; // just one response by request!
			_pThread = _session.send<HTTPSender>(ex, qos(),pSender,_pThread);
			if (ex)
				ERROR("Pushing HTTPSender flush, ", ex.error())
			if (pSender->written())
				one=true;
			_pushSenders.pop_front();
		}
	}
}

void HTTPWriter::writeFile(const Path& file, DataReader& parameters) {
	HTTPSender* pSender(createSender());
	if (!pSender) {
		ERROR("No HTTP request to send file ",file.name())
		return;
	}
	return pSender->writeFile(file,parameters);
}	


DataWriter& HTTPWriter::write(const string& code, HTTP::ContentType type, const char* subType, const UInt8* data,UInt32 size) {
	if(state()==CLOSED)
        return DataWriter::Null;

	HTTPSender* pSender(createSender());
	if (!pSender) {
		ERROR("No HTTP request to send this reply")
		return DataWriter::Null;
	}

	return pSender->writer(code, type, subType, data, size);
}

DataWriter& HTTPWriter::writeMessage() {
	if (_pRequest && _pRequest->contentType != HTTP::CONTENT_ABSENT)
		return write("200 OK", _pRequest->contentType, _pRequest->contentSubType.c_str(),NULL,_pRequest->rawSerialization ? 2 : 0);
	return write("200 OK", HTTP::CONTENT_APPLICATON , "json");
}

bool HTTPWriter::writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties) {
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
			if (!_pRequest)
				ex.set(Exception::APPLICATION, "HTTP streaming without request related");
			else if (_pRequest->contentSubType.compare(0, 5, "x-flv")==0)
				_pMedia.reset(new FLV());
			else if(_pRequest->contentSubType.compare(0,4,"mpeg")==0)
				_pMedia.reset(new MPEGTS());
			else
				ex.set(Exception::APPLICATION, "HTTP streaming for a ",_pRequest->contentSubType," unsupported");
			if (ex) {
				close(ex);
				return false;
			}
			// write a HTTP header without content-length (data==NULL and size==1)
			_pMedia->write(write("200", _pRequest->contentType, _pRequest->contentSubType.c_str(), NULL, 1).packet);
			break;
		}
		case AUDIO:
		case VIDEO: {
			if (!_pMedia)
				return false;
			HTTPSender* pSender(createSender());
			if (!pSender)
				return false;
			_pMedia->write(pSender->writeRaw(_session.invoker.poolBuffers),type,time,packet.current(), packet.available());
			break;
		}
		default:
			return Writer::writeMedia(type,time,packet,properties);
	}
	return true;
}


} // namespace Mona
