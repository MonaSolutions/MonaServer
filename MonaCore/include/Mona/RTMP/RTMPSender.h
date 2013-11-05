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
#include "Mona/TCPSender.h"
#include "Mona/AMFWriter.h"
#include "Mona/MemoryReader.h"
#include "Mona/RTMP/RTMP.h"


namespace Mona {

class RTMPSender : public TCPSender, virtual Object {
public:
	RTMPSender(const std::shared_ptr<RC4_KEY>& pEncryptKey);
	RTMPSender(const RTMPSender& sender);

	AMFWriter	writer;
	
	AMFWriter&	write(UInt32 id,AMF::ContentType type,UInt32 time=0,UInt32 streamId=0,MemoryReader* pData=NULL);
	
private:
	bool				run(Exception& ex);
	void				pack();
	const UInt8*		begin(bool displaying = false) { return writer.stream.data(); }
	UInt32				size(bool displaying = false) { return writer.stream.size(); }

	UInt32						_sizePos;
	UInt32						_chunkSize;
	RTMPChannel					_channel;
	std::shared_ptr<RC4_KEY>	_pEncryptKey;
};


} // namespace Mona
