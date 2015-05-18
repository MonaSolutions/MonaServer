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
#include "Mona/DataWriter.h"
#include "Mona/QualityOfService.h"
#include "Mona/PacketReader.h"
#include "Mona/Parameters.h"
#include "Mona/Event.h"


namespace Mona {

namespace Events {
	struct OnClose : Event<void(Int32 code)> {};
};

class Client;
class Writer : virtual public NullableObject,
	public Events::OnClose {
public:
	enum MediaType {
		INIT,
		AUDIO,
		VIDEO,
		DATA,
		START,
		STOP
	};
	enum DataType {
		DATA_USER=1,
		DATA_INFO=2
	};
	enum State {
		OPENING,
		OPENED,
		CLOSED
	};

	bool					reliable;


	const QualityOfService&	qos() const { return _qos; }


	virtual Writer&			newWriter() { return *this; }

	virtual void			clear() {} // must erase the queueing messages (don't change the writer state)

	State					state() { return _state; }
	void					open() { if(_state==OPENING) _state = OPENED;}
	
	/**	The main Writer of one session should close the entiere session
		If code==0, it's a normal close
		If code>0, it's a user close (from server application script)
		If code<0, it's a system core close
			-1 => Listener close!				*/
	virtual void			close(Int32 code=0);

	/**	Call  by Multimedia framework on a writer Listener after subscription.
		In first, for every media type (AUDIO, VIDEO, DATA), an type=INIT call is invoked on initialization with time=MediaType,
		and publication name in packet.
		If this call returns false, the writer can be closed, and subscription have to be removed by the writer handler.
		Finally, every media data are passed (AUDIO, VIDEO and DATA), if the methods returns false, the cycle restart since the beginning
		For DATA type, time is MIME::Type on 3 bytes and Writer::DataType on 1 byte */
	virtual bool			writeMedia(MediaType type,UInt32 time,PacketReader& packet,const Parameters& properties);

	virtual bool			writeMember(const Client& client);

    virtual DataWriter&		writeInvocation(const char* name){return DataWriter::Null;}
    virtual DataWriter&		writeMessage(){return DataWriter::Null;}
	virtual DataWriter&		writeResponse(UInt8 type){return writeMessage();}
	virtual void			writeRaw(const UInt8* data,UInt32 size){}

	virtual bool			flush() { return false;  } // return true if something has been sent!

	operator bool() const { return !_isNull; }

    static Writer			Null;

protected:
	Writer(State state);
	Writer(const Writer& other);
	virtual ~Writer();

	QualityOfService				_qos;
	
private:
	Writer();

	State							_state;
	const bool						_isNull;
};


} // namespace Mona
