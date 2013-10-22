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
#include "Mona/FlashStream.h"

namespace Mona {

class FlashMainStream : public FlashStream {
public:
	FlashMainStream(Invoker& invoker,Peer& peer);
	virtual ~FlashMainStream();

private:

	void	messageHandler(Exception& ex, const std::string& name,AMFReader& message,FlashWriter& writer);
	void	rawHandler(Mona::UInt8 type,MemoryReader& data,FlashWriter& writer);

	void	close(FlashWriter& writer,const std::string& error,int code=0);

	std::set<Mona::UInt32>	_streams;
	Group*					_pGroup;
};


} // namespace Mona
