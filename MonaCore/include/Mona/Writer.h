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
#include "Mona/DataReader.h"
#include "Mona/QualityOfService.h"
#include "Mona/MemoryReader.h"
#include "Poco/SharedPtr.h"

namespace Mona {

class Writer;
class WriterHandler {	
public:
	virtual void	close(Writer& writer,int code){}
};

class Peer;
class Writer {
public:
	enum MediaType {
		INIT=0,
		AUDIO,
		VIDEO,
		DATA,
		START,
		STOP
	};
	enum State {
		GET,
		CONNECTING,
		CONNECTED,
		CLOSED
	};

	bool					reliable;

	const QualityOfService&	qos();


	virtual Writer&			newWriter(WriterHandler* pHandler=NULL);

	virtual State			state(State value=GET,bool minimal=false);
	virtual void			close(int code=0);

	virtual bool			writeMedia(MediaType type,UInt32 time,MemoryReader& data);
	virtual void			writeMember(const Peer& peer);

	virtual DataWriter&		writeInvocation(const std::string& name){return DataWriterNull;}
	virtual DataWriter&		writeMessage(){return DataWriterNull;}
	virtual void			writeRaw(const UInt8* data,UInt32 size){}

	virtual void			flush(bool full=false){}

	virtual void			createReader(MemoryReader& reader,Poco::SharedPtr<DataReader>& pReader) {}
	virtual void			createWriter(Poco::SharedPtr<DataWriter>& pWriter) {}
	virtual bool			hasToConvert(DataReader& reader) {return false;}


	static Writer				Null;
	static DataWriterNull	DataWriterNull;

protected:
	Writer(WriterHandler* pHandler=NULL);
	Writer(Writer& writer);
	virtual ~Writer();

	QualityOfService		_qos;
	WriterHandler*			_pHandler;
	State					_state;
};

inline Writer::State Writer::state(Writer::State value,bool minimal) {
	if(value==GET)
		return _state;
	return _state=value;
}

inline Writer& Writer::newWriter(WriterHandler* pHandler) {
	return *this;
}

inline const QualityOfService& Writer::qos() {
	return _qos;
}


} // namespace Mona
