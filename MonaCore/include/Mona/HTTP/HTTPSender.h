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
#include "Mona/HTTPPacketWriter.h"


namespace Mona {

class HTTPSender : public TCPSender {
public:
	HTTPSender(SocketHandler<Poco::Net::StreamSocket>& handler) : TCPSender(handler,true) {}
	virtual ~HTTPSender() {}
	
	HTTPPacketWriter	writer;
private:

	const Mona::UInt8*	begin(bool displaying=false);
	Mona::UInt32		size(bool displaying=false);

};

inline const Mona::UInt8* HTTPSender::begin(bool displaying) {
	return	writer.stream.data();
}

inline Mona::UInt32 HTTPSender::size(bool displaying) {
	return writer.stream.size();
}



} // namespace Mona
