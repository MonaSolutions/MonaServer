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
#include "Mona/FlashWriter.h"
#include "Mona/Invoker.h"

namespace Mona {

namespace FlashEvents {
	struct OnStart : Event<void(UInt16 id, FlashWriter& writer)> {};
	struct OnStop : Event<void(UInt16 id, FlashWriter& writer)> {};
};

class FlashStream : public virtual Object,
	public FlashEvents::OnStart,
	public FlashEvents::OnStop {
public:
	FlashStream(UInt16 id, Invoker& invoker,Peer& peer);
	virtual ~FlashStream();

	const UInt16	id;

	UInt32	bufferTime(UInt32 ms);
	UInt32	bufferTime() const { return _bufferTime; }

	void	disengage(FlashWriter* pWriter=NULL);

	// return flase if writer is closed!
	bool	process(AMF::ContentType type,UInt32 time,PacketReader& packet,FlashWriter& writer,double lostRate=0);

	virtual void	flush() { if(_pPublication) _pPublication->flush(); }

protected:

	Invoker&		invoker;
	Peer&			peer;

private:

	virtual void	messageHandler(const std::string& name, AMFReader& message, FlashWriter& writer);
	virtual void	rawHandler(UInt16 type, PacketReader& data, FlashWriter& writer);
	virtual void	dataHandler(DataReader& data, double lostRate);
	virtual void	audioHandler(UInt32 time, PacketReader& packet, double lostRate);
	virtual void	videoHandler(UInt32 time,PacketReader& packet, double lostRate);

	Publication*	_pPublication;
	Listener*		_pListener;
	UInt32			_bufferTime;
};


} // namespace Mona
