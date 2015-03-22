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
#include "Mona/WebSocket/WSSession.h"
#include "Mona/HTTP/HTTPWriter.h"
#include "Mona/HTTP/HTTPPacket.h"
#include "Mona/HTTP/HTTPDecoder.h"

namespace Mona {

class HTTPPacketReader;
class QueryReader;

class HTTPSession :  public WSSession {
public:
	HTTPSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker);
	
private:
	const char*		cookie(const char* key);

	void			kill(UInt32 type=NORMAL_DEATH);
	void			manage();

	UInt32			onData(PoolBuffer& pBuffer) { if (WSSession::enabled()) return WSSession::onData(pBuffer); return _decoder.decode(pBuffer); }
	void			receive(const std::shared_ptr<HTTPPacket>& pPacket);

	/// \brief Send the Option response
	/// Note: It is called when processMove is used before a SOAP request
	void			processOptions(Exception& ex,const HTTPPacket& request);

	/// \brief Process GET & HEAD commands
	/// Search for a method or a file whitch correspond to the _filePath
	void			processGet(Exception& ex, HTTPPacket& request, QueryReader& parameters);

	HTTPDecoder::OnDecoded::Type	onDecoded;
	HTTPDecoder::OnDecodedEnd::Type	onDecodedEnd;


	HTTPDecoder			_decoder;

	HTTPWriter			_writer;

	// options
	std::string			_index;
	bool				_indexDirectory;

	Client::OnCallProperties::Type	onCallProperties; /// client.properties() handler => add cookie to session
};


} // namespace Mona
