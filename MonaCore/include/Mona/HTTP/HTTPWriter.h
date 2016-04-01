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

	void			beginRequest(const std::shared_ptr<HTTPPacket>& pRequest);
	void			endRequest();

	const HTTPPacket* lastRequest() const { return _pLastRequest ? &*_pLastRequest : NULL; }

	bool			writeSetCookie(DataReader& reader,const HTTP::OnCookie& onCookie=nullptr) { return HTTP::WriteSetCookie(reader,*_pSetCookieBuffer, onCookie); }

	void			clear() { _pResponse.reset(); _senders.clear(); }

	DataWriter&		writeInvocation(const char* name) { DataWriter& writer(writeMessage()); writer.writeString(name,strlen(name)); return writer; }
	DataWriter&		writeMessage();
	DataWriter&		writeResponse(UInt8 type);
	void			writeRaw(const UInt8* data, UInt32 size);
	void			close(Int32 code=0);

	/// \brief create a Sender and write the file in parameter
	/// \param file path of the file
	void			writeFile(const std::string& path, const std::shared_ptr<Parameters>& pParameters);
	void			close(const Exception& ex);

	DataWriter&     writeRaw(const char* code);

private:
	bool			flush();

	bool			writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties);

	HTTPSender*     createSender(bool isInternResponse);
	bool			send(std::shared_ptr<HTTPSender>& pSender);

	std::unique_ptr<MediaContainer>				_pMedia;
	TCPSession&									_session;
	PoolThread*									_pThread;
	std::shared_ptr<HTTPSender>					_pResponse;
	std::deque<std::shared_ptr<HTTPSender>>		_senders;
	PoolBuffer									_pSetCookieBuffer;
	bool										_isMain;
	std::string									_lastError;
	std::shared_ptr<HTTPPacket>					_pRequest;
	UInt32										_requestCount;
	std::shared_ptr<HTTPPacket>					_pLastRequest;
	bool										_requesting;
};



} // namespace Mona
