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
#include "Mona/Peer.h"
#include "Mona/Protocol.h"
#include "Mona/Invoker.h"


namespace Mona {

class Session : virtual Object {
public:

	const UInt32		id;
	const std::string	name;
	const bool			died;

	Peer				peer;
	const bool			checked;
	bool				failed;

	virtual ~Session();

	void						receive(MemoryReader& packet);
	virtual bool				decode(const std::shared_ptr < Buffer < UInt8 >> &pBuffer, const SocketAddress& address, std::shared_ptr <MemoryReader>& pReader) { pReader.reset(new MemoryReader(pBuffer->data(), pBuffer->size())); return true; }
	virtual void				manage() {}
	virtual void				kill();

protected:
	Session(Protocol& protocol,Invoker& invoker, const char* name=NULL);
	Session(Protocol& protocol,Invoker& invoker, const Peer& peer,const char* name=NULL);

	Invoker&			invoker;
	Protocol&			protocol;

	template<typename DecodingType>
	bool decode(const std::shared_ptr<DecodingType>& pDecoding) {
		Exception ex;
		_pDecodingThread = invoker.poolThreads.enqueue<DecodingType>(ex,pDecoding, _pDecodingThread);
		if (ex)
			ERROR("Impossible to decode packet of protocol ", protocol.name, " on session ", id, ", ", ex.error());
		return !ex;
	}

	const std::string& reference();

	virtual void		packetHandler(MemoryReader& packet)=0;

private:	
	PoolThread*			_pDecodingThread;
};


} // namespace Mona
