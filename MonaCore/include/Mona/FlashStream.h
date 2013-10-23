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
#include "Mona/AMF.h"
#include "Mona/AMFReader.h"
#include "Mona/Peer.h"
#include "Mona/MemoryReader.h"
#include "Mona/FlashWriter.h"

namespace Mona {

class Invoker;
class FlashStream {
public:
	FlashStream(UInt32 id,Invoker& invoker,Peer& peer);
	virtual ~FlashStream();

	const UInt32	id;
	
	void setBufferTime(UInt32 ms);

	void process(AMF::ContentType type,MemoryReader& data,FlashWriter& writer,UInt32 numberLostFragments=0);
	void flush();

protected:

	Invoker&	invoker;
	Peer&		peer;

private:
	void			disengage(FlashWriter* pWriter=NULL);

	virtual void	messageHandler(Exception& ex, const std::string& name, AMFReader& message, FlashWriter& writer);
	virtual void	rawHandler(Exception& ex, UInt8 type, MemoryReader& data, FlashWriter& writer);
	virtual void	dataHandler(Exception& ex, DataReader& data, UInt32 numberLostFragments);
	virtual void	audioHandler(Exception& ex, MemoryReader& packet, UInt32 numberLostFragments);
	virtual void	videoHandler(Exception& ex, MemoryReader& packet, UInt32 numberLostFragments);

	virtual void	close(FlashWriter& writer,const std::string& error,int code=0);

	Publication*	_pPublication;
	Listener*		_pListener;
	UInt32			_bufferTime;
};


} // namespace Mona
