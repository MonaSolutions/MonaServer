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
#include "Mona/Writer.h"
#include "Mona/HTTP/HTTPSender.h"
#include "Mona/TCPClient.h"
#include "Mona/MediaContainer.h"

namespace Mona {



class HTTPWriter : public Writer, virtual Object {
public:

	HTTPWriter(TCPClient& tcpClient);

	std::shared_ptr<HTTPPacket>		pRequest;
	Time							timeout;

	virtual State			state(State value=GET,bool minimal=false);
	virtual void			flush(bool full=false);

	virtual DataWriter&		writeInvocation(const std::string& name) { DataWriter& writer = write("200 OK", contentType, contentSubType); writer.writeString(name); return writer; }
	virtual DataWriter&		writeMessage() { return write("200 OK", contentType, contentSubType); }
	virtual DataWriter&		writeResponse(UInt8 type);
	virtual void			writeRaw(const UInt8* data, UInt32 size) { write("200 OK", HTTP::CONTENT_TEXT,"plain; charset=utf-8",data,size); }
	virtual void			close(int code=0);

	DataWriter&		write(const std::string& code, HTTP::ContentType type=HTTP::CONTENT_TEXT, const std::string& subType="html; charset=utf-8",const UInt8* data=NULL,UInt32 size=0);

	/// \brief create a Sender and write the file in parameter
	/// \param file path of the file
	/// \param sortOptions Sort options for directory listing
	/// \param isApp True if file is an application
	void			writeFile(const FilePath& file, UInt8 sortOptions, bool isApp) { return createSender().writeFile(file,sortOptions,isApp);}
	void			close(const Exception& ex);
	
	
	std::unique_ptr<MediaContainer>		media;
	HTTP::ContentType					contentType; ///< Content type for pull response
	std::string							contentSubType; ///< Content sub type for pull response 
private:
	bool			writeMedia(MediaType type,UInt32 time,PacketReader& packet);
	
	HTTPSender& createSender() {
		_senders.emplace_back(new HTTPSender(_tcpClient.address(),pRequest));
		return *_senders.back();
	}

	TCPClient&									_tcpClient;
	PoolThread*									_pThread;
	std::vector<std::shared_ptr<HTTPSender>>	_senders;
	bool										_isMain;
	std::string									_buffer;
};



} // namespace Mona
