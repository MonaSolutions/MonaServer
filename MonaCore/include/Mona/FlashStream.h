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
#include "Mona/AMF.h"
#include "Mona/AMFReader.h"
#include "Mona/Peer.h"
#include "Mona/FlashWriter.h"

namespace Mona {

class Invoker;
class FlashStream : public virtual Object {
public:
	FlashStream(UInt32 id,Invoker& invoker,Peer& peer);
	virtual ~FlashStream();

	const UInt32	id;
	
	void setBufferTime(UInt32 ms);

	// return flase if writer is closed!
	bool		 process(AMF::ContentType type,UInt32 time,PacketReader& packet,FlashWriter& writer,UInt32 numberLostFragments=0);
	virtual void flush();


	virtual FlashStream* stream(UInt32 id) { return NULL; }

protected:

	Invoker&	invoker;
	Peer&		peer;

private:
	void			disengage(FlashWriter* pWriter=NULL);

	virtual void	messageHandler(const std::string& name, AMFReader& message, FlashWriter& writer);
	virtual void	rawHandler(UInt8 type, PacketReader& data, FlashWriter& writer);
	virtual void	dataHandler(DataReader& data, UInt32 numberLostFragments);
	virtual void	audioHandler(UInt32 time, PacketReader& packet, UInt32 numberLostFragments);
	virtual void	videoHandler(UInt32 time,PacketReader& packet, UInt32 numberLostFragments);

	Publication*	_pPublication;
	Listener*		_pListener;
	UInt32			_bufferTime;
};


} // namespace Mona
