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

HTTPWriter::HTTPWriter(TCPSession& session) : _pSetCookieBuffer(session.invoker.poolBuffers),_requestCount(0),_requesting(false),_session(session),_pThread(NULL),Writer(session.peer.connected ? OPENED : OPENING) {
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
		default: break;
	}
	_lastError.assign(ex.error());
	close(code);
}

void HTTPWriter::close(Int32 code) {
	if (code < 0)
		return; // listener!
	HTTPSender* pSender;
	if (code > 0 && (pSender=createSender(true)))
		pSender->writeError(_lastError,code);
	else
		_session.kill(code); // kill just if no message is send
	Writer::close(code);
}

void HTTPWriter::beginRequest(const shared_ptr<HTTPPacket>& pRequest) {
	++_requestCount;
	_pRequest = pRequest;
	_pLastRequest = pRequest;
	_requesting = true;
}

void HTTPWriter::endRequest() {
	_requesting = false;
	flush();
}

HTTPSender* HTTPWriter::createSender(bool isInternResponse) {
	if(state()==CLOSED)
		return NULL;
	if (isInternResponse) {
		if (!_pRequest)
			return NULL;
		if (_pResponse)
			ERROR("HTTP Response already written")
		_pResponse.reset(new HTTPSender(_session.peer.address, *_pRequest, _session.invoker.poolBuffers, _session.peer.path,_pSetCookieBuffer));
		return &*_pResponse;
	}
	if (!_pRequest && !_pLastRequest) {
		ERROR("No HTTP request to answer")
		return NULL;
	}
	_senders.emplace_back(new HTTPSender(_session.peer.address,_pRequest ? *_pRequest : *_pLastRequest,_session.invoker.poolBuffers,_session.peer.path,_pSetCookieBuffer));
	return &*_senders.back();
}

bool HTTPWriter::send(shared_ptr<HTTPSender>& pSender) {
	Exception ex;
	if (pSender->newHeaders()) {
		if (!_requestCount)
			return false;
		if(--_requestCount==0)
			_pRequest.reset();
	}
	_pThread = _session.send<HTTPSender>(ex, qos(),pSender,_pThread);
	if (ex)
		ERROR("HTTPSender flush, ", ex.error())
	pSender.reset();
	return true;
}

bool HTTPWriter::flush() {

	if (_requesting) // during request wait the main response before flush
		return false;

	// now send just one response with header!
	if (_pResponse && !send(_pResponse))
		return false;

	// send senders
	while (!_senders.empty() && send(_senders.front()))
		_senders.pop_front();
	return true;
}

DataWriter& HTTPWriter::writeMessage() {
	HTTPSender* pSender(createSender(false));
	if (!pSender)
		return DataWriter::Null;
	return pSender->writeResponse();
}

DataWriter& HTTPWriter::writeResponse(UInt8 type) {
	HTTPSender* pSender(createSender(true));
	if (!pSender)
		return DataWriter::Null;
	return pSender->writeResponse();
}

void HTTPWriter::writeFile(const string& path, const shared_ptr<Parameters>& pParameters) {
	HTTPSender* pSender(createSender(false));
	if (pSender)
		pSender->writeFile(path,pParameters);
}	

void HTTPWriter::writeRaw(const UInt8* data, UInt32 size) {
	HTTPSender* pSender(createSender(false));
	if (pSender)
		pSender->write("200 OK", HTTP::CONTENT_TEXT,"plain",data,size); 
}	

DataWriter& HTTPWriter::writeRaw(const char* code) {
	HTTPSender* pSender(createSender(true));
	if (pSender)
		return pSender->write(code, HTTP::CONTENT_ABSENT, NULL);
	return DataWriter::Null;
}

bool HTTPWriter::writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties) {
	if(state()==CLOSED)
		return true;
	switch(type) {
		case START:
		case STOP:
			break;
		case INIT: {
			if (time!=DATA) // one init by mediatype, we want here just init one time!
				break;
			Exception ex;
			if (!_pRequest)
				ex.set(Exception::APPLICATION, "HTTP streaming without request related");
			else if (_pRequest->contentSubType.compare(0, 5, "x-flv")==0)
				_pMedia.reset(new FLV(properties,_session.invoker.poolBuffers));
			else if(_pRequest->contentSubType.compare(0,4,"mp2t")==0)
				_pMedia.reset(new MPEGTS(_session.invoker.poolBuffers));
			else
				ex.set(Exception::APPLICATION, "HTTP streaming for a ",_pRequest->contentSubType," unsupported");

			if (ex) {
				close(ex);
				return false;
			}
			// write a HTTP header without content-length (data==NULL and size==1)
			_pMedia->write(createSender(true)->writeResponse("200 OK",true).packet);
			break;
		}
		case AUDIO:
		case VIDEO: {
			if (!_pMedia)
				return false;
			HTTPSender* pSender(createSender(false));
			if (!pSender)
				return false;
			_pMedia->write(pSender->writeRaw(),type,time,packet.current(), packet.available());
			break;
		}
		case DATA: {
			if (!_pMedia)
				return false;
			HTTPSender* pSender(createSender(false));
			if (!pSender)
				return false;
			_pMedia->write(pSender->writeRaw(),(Writer::DataType)(time & 0xFF),(MIME::Type)(time >> 8),packet);
			break;
		}
		default:
			return Writer::writeMedia(type,time,packet,properties);
	}
	return true;
}


} // namespace Mona
