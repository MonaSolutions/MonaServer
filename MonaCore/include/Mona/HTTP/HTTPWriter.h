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
#include "Mona/Writer.h"
#include "Mona/HTTP/HTTPSender.h"
#include "Mona/MediaContainer.h"

namespace Mona {



class HTTPWriter : public Writer, public virtual Object {
public:

	HTTPWriter(TCPSession& session);

	const std::shared_ptr<HTTPPacket>&	setRequest(const std::shared_ptr<HTTPPacket>& pRequest) { _pRequest = pRequest; if (!_pFirstRequest) _pFirstRequest = pRequest; return _pRequest; }
	HTTPPacket*							request() { return _pRequest ? &*_pRequest : NULL; }

	void			abort() { _pSender.reset(); _pushSenders.clear();  }
	void			flush(bool withPush);

	DataWriter&		writeInvocation(const char* name) { DataWriter& writer(writeMessage()); writer.writeString(name,strlen(name)); return writer; }
	DataWriter&		writeMessage();
	DataWriter&		writeResponse(UInt8 type) { return writeMessage(); }
	void			writeRaw(const UInt8* data, UInt32 size) { write("200 OK", HTTP::CONTENT_TEXT,"plain",data,size); }
	void			close(Int32 code=0);

	DataWriter&		write(const std::string& code, HTTP::ContentType type=HTTP::CONTENT_ABSENT, const char* subType=NULL,const UInt8* data=NULL,UInt32 size=0);

	/// \brief create a Sender and write the file in parameter
	/// \param file path of the file
	void			writeFile(const Path& file, DataReader& parameters);

	void			close(const Exception& ex);

private:
	bool			writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties);
	void			flush() { flush(false); }
	
	HTTPSender*     createSender();

	std::unique_ptr<MediaContainer>				_pMedia;
	TCPSession&									_session;
	PoolThread*									_pThread;
	std::shared_ptr<HTTPSender>					_pSender;
	std::deque<std::shared_ptr<HTTPSender>>		_pushSenders;
	bool										_isMain;
	std::string									_lastError;
	std::shared_ptr<HTTPPacket>					_pRequest;
	std::shared_ptr<HTTPPacket>					_pFirstRequest;
};



} // namespace Mona
