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
#include "Mona/FlashWriter.h"
#include "Mona/RTMP/RTMPSender.h"

namespace Mona {


class RTMPWriter : public FlashWriter, public virtual Object {
public:
	RTMPWriter(UInt32 id,TCPSession& session,std::shared_ptr<RTMPSender>& pSender,const std::shared_ptr<RC4_KEY>& pEncryptKey);
	virtual ~RTMPWriter() { if (channel.pStream) channel.pStream->disengage(this); }

	const UInt32	id;
	RTMPChannel		channel; // Input channel
	bool			isMain;

	void			clear() { if(_pSender) _pSender.reset(); FlashWriter::clear(); }
	void			close(Int32 code=0);

	bool			flush();

	void			writeAck(UInt32 count) {write(AMF::ACK).packet.write32(count);}
	void			writeWinAckSize(UInt32 value) {write(AMF::WIN_ACKSIZE).packet.write32(value);}
	void			writeProtocolSettings();
private:
	RTMPWriter(const RTMPWriter& other) = delete; // require by gcc 4.8 to build _writers of RTMPSession

	AMFWriter&		write(AMF::ContentType type,UInt32 time=0,const UInt8* data=NULL,UInt32 size=0);
	
	RTMPChannel						_channel; // Output channel
	std::shared_ptr<RTMPSender>&	_pSender;
	TCPSession&						_session;
	const std::shared_ptr<RC4_KEY>	_pEncryptKey;
	
};



} // namespace Mona
