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
#include "Mona/FlashWriter.h"
#include "Mona/RTMP/RTMPSender.h"

namespace Mona {



class RTMPWriter : public FlashWriter, virtual Object {
public:
	RTMPWriter(UInt8 id,const Poco::SharedPtr<RC4_KEY>& pEncryptKey,StreamSocket& socket);

	const UInt8		id;
	RTMPChannel		channel;


	State			state(State value=GET,bool minimal=false);
	void			close(int code=0);

	void			writeRaw(const UInt8* data,UInt32 size);

	void			flush(bool full=false);

	void			writeMaxChunkSize();
private:
	AMFWriter&		write(AMF::ContentType type,UInt32 time=0,MemoryReader* pData=NULL);

	std::shared_ptr<RTMPSender>	_pSender;
	StreamSocket&				_socket;
	PoolThread*					_pThread;
};



} // namespace Mona
