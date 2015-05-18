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
#include "Mona/FlashStream.h"

namespace Mona {

class FlashMainStream : public FlashStream, public virtual Object {
public:
	FlashMainStream(Invoker& invoker,Peer& peer);
	virtual ~FlashMainStream();

	void	disengage(FlashWriter* pWriter = NULL);

	FlashStream* getStream(UInt16 id, std::shared_ptr<FlashStream>& pStream);

	void flush() {for(auto& it : _streams) it.second->flush(); }
	
private:
	void	messageHandler(const std::string& name, AMFReader& message, FlashWriter& writer);
	void	rawHandler(UInt16 type, PacketReader& packet, FlashWriter& writer);

	std::map<UInt16,std::shared_ptr<FlashStream>>	_streams;
	std::string										_buffer;
};


} // namespace Mona
