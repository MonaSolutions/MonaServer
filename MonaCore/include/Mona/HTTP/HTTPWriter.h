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
	
	std::shared_ptr<HTTPPacket>		pRequest;

	virtual void			abort() {_senders.clear();}
	virtual void			flush();

	virtual DataWriter&		writeInvocation(const char* name) { DataWriter& writer(writeMessage()); writer.writeString(name,strlen(name)); return writer; }
	virtual DataWriter&		writeMessage();
	virtual DataWriter&		writeResponse(UInt8 type) { return writeMessage(); }
	virtual void			writeRaw(const UInt8* data, UInt32 size) { write("200 OK", HTTP::CONTENT_TEXT,"plain",data,size); }
	virtual void			close(Int32 code=0);

	DataWriter&		write(const std::string& code, HTTP::ContentType type=HTTP::CONTENT_ABSENT, const char* subType=NULL,const UInt8* data=NULL,UInt32 size=0);

	/// \brief create a Sender and write the file in parameter
	/// \param file path of the file
	/// \param sortOptions Sort options for directory listing
	/// \param isApp True if file is an application
	void			writeFile(const Path& file, UInt8 sortOptions, bool isApp);

	void			close(const Exception& ex);

private:
	bool			writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties);
	
	HTTPSender& createSender(HTTPPacket& request) { return **_senders.emplace(_senders.end(),new HTTPSender(_session.peer.address,request,_session.invoker.poolBuffers,_session.peer.path)); }

	std::unique_ptr<MediaContainer>				_pMedia;
	TCPSession&									_session;
	PoolThread*									_pThread;
	std::vector<std::shared_ptr<HTTPSender>>	_senders;
	bool										_isMain;
	std::string									_lastError;
};



} // namespace Mona
