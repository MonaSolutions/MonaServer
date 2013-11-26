/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
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
#include "Mona/WebSocket/WSSession.h"
#include "Mona/HTTP/HTTPWriter.h"


namespace Mona {

class HTTPPacketReader;

class HTTPSession :  public WSSession {
public:

	HTTPSession(const SocketAddress& address, Protocol& protocol, Invoker& invoker);
	virtual ~HTTPSession();
	
private:
	void			manage();

	bool			buildPacket(const std::shared_ptr<Buffer<UInt8>>& pData, MemoryReader& packet);
	void			packetHandler(MemoryReader& packet);
	void			endReception() { if (_isWS) WSSession::endReception(); }

	/// \brief Send the HTTP error response
	void			sendError(Exception& ex);
	
	/// \brief Send the file
	void			processGet(Exception& ex, const std::string& fileName);

	/// \brief Parse SOAP request, execute lua function and send SOAP response
	void			processSOAPfunction(Exception& ex, MemoryReader& packet);

	/// \brief Send the Option response
	/// Note: It is called when processMove is used before a SOAP request
	void			processOptions(Exception& ex, const std::string& methods);

	/// \brief Send the Move response if the file requested is a directory
	void			processMove(const std::string& filePath);

	/// \brief Send Not Modified
	void			processNotModified();

	std::string			_cmd;
	std::string			_file;
	std::unique_ptr<HTTPPacketReader> _request;
	MapParameters	_headers;
	HTTPWriter		_writer;
	bool			_isWS;
};


} // namespace Mona
