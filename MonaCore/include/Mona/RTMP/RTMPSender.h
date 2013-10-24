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
#include "Poco/SharedPtr.h"

namespace Mona {

class RTMPSender : public TCPSender {
public:
	RTMPSender(const Poco::SharedPtr<RC4_KEY>& pEncryptKey,SocketHandler<Poco::Net::StreamSocket>& handler);
	RTMPSender(const RTMPSender& sender);
	virtual ~RTMPSender();

	AMFWriter	writer;
	
	AMFWriter&	write(UInt32 id,AMF::ContentType type,UInt32 time=0,UInt32 streamId=0,MemoryReader* pData=NULL);
	
private:

	void				pack();
	bool				flush();
	const UInt8*	begin(bool displaying=false);
	UInt32		size(bool displaying=false);

	UInt32				_sizePos;
	UInt32				_chunkSize;
	RTMPChannel					_channel;
	Poco::SharedPtr<RC4_KEY>	_pEncryptKey;
};

inline const UInt8* RTMPSender::begin(bool displaying) {
	return writer.stream.data();
}

inline UInt32 RTMPSender::size(bool displaying) {
	return writer.stream.size();
}



} // namespace Mona
