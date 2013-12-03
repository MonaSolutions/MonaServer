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
#include "Mona/TCPSender.h"
#include "Mona/AMFWriter.h"
#include "Mona/RTMP/RTMP.h"


namespace Mona {

class RTMPSender : public TCPSender, virtual Object {
public:
	RTMPSender(const std::shared_ptr<RC4_KEY>& pEncryptKey=nullptr) : _pEncryptKey(pEncryptKey),sizePos(0),headerSize(0),TCPSender("RTMPSender") {}
	RTMPSender(const RTMPSender& sender) : sizePos(0),headerSize(0),TCPSender("RTMPSender"),_pEncryptKey(sender._pEncryptKey) {}

	UInt32				sizePos;
	UInt8				headerSize;

	const UInt8*		begin() { return _writer.stream.data(); }
	UInt32				size() { return _writer.stream.size(); }

	bool				encrypted() { return _pEncryptKey ? true : false; }

	void clear() { _writer.clear(); }

	AMFWriter& writer(RTMPChannel& channel);
private:
	bool				run(Exception& ex);

	AMFWriter					_writer;
	std::shared_ptr<RC4_KEY>	_pEncryptKey;
};


} // namespace Mona
