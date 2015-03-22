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
#include "Mona/WebSocket/WSWriter.h"
#include "Mona/WebSocket/WSDecoder.h"


namespace Mona {


class WSSession : public TCPSession, public virtual Object {
public:

	WSSession(const SocketAddress& peerAddress, SocketFile& file, Protocol& protocol, Invoker& invoker);

	void			flush() { if (_pPublication) _pPublication->flush(); Session::flush(); }
	void			manage();


protected:
	bool			enabled() { return _enabled; }
	void			enable();

	WSWriter&		wsWriter() { return _writer; }
	void			kill(UInt32 type=NORMAL_DEATH);

	bool			openSubscribtion(Exception& ex, const std::string& name, Writer& writer);
	void			closeSusbcription();
	
	bool			openPublication(Exception& ex, const std::string& name, Publication::Type type);
	void			closePublication();

	UInt32			onData(PoolBuffer& pBuffer) { return _decoder.decode(pBuffer); }

	/// \brief Read message and call method if needed
	/// \param packet Content message to read
	void			readMessage(Exception& ex, DataReader& reader, UInt8 responseType=0);

private:
	void			receive(WSReader& packet);

	WSDecoder::OnDecoded::Type		onDecoded;
	WSDecoder::OnDecodedEnd::Type	onDecodedEnd;

	WSWriter		_writer;
	WSDecoder		_decoder;
	bool			_enabled;

	Publication*	_pPublication;
	Listener*		_pListener;
};


} // namespace Mona
