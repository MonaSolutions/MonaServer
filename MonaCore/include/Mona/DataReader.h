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
#include "Mona/MemoryReader.h"
#include "Mona/DataWriter.h"
#include "Mona/Time.h"

namespace Mona {

class DataReader : ObjectFix {
public:
	enum Type {
		NIL=0,
		BOOLEAN,
		NUMBER,
		STRING,
		TIME,
		ARRAY,
		OBJECT,
		BYTES,
		MAP,
		END
	};

	virtual ~DataReader();
		
	virtual Type				followingType()=0;

	virtual void				readString(std::string& value)=0;
	virtual double				readNumber()=0;
	virtual bool				readBoolean()=0;
	virtual const UInt8*		readBytes(UInt32& size)=0;
	virtual Time&				readTime(Time& time) = 0;
	virtual void				readNull()=0;

	virtual bool				readObject(std::string& type,bool& external)=0;
	virtual bool				readArray(UInt32& size)=0;
	virtual Type				readItem(std::string& name)=0;

	virtual bool				readMap(UInt32& size,bool& weakKeys);
	virtual Type				readKey() { return followingType(); }
	virtual Type				readValue() { return followingType(); }

	virtual bool				available() { return reader.available() > 0; }

	virtual void				reset() { reader.reset(_pos); }

	void						next();
	void						read(DataWriter& writer,UInt32 count=0);

	MemoryReader&				reader;

protected:
	DataReader(MemoryReader& reader);
private:
	void						read(Type type,DataWriter& writer);

	UInt32				_pos;
};


class DataReaderNull : public DataReader {
public:
	DataReaderNull() : DataReader(MemoryReader::Null) {}

	Type followingType() {return END;}

	void						readString(std::string& value){}
	virtual double				readNumber(){return 0;}
	virtual bool				readBoolean(){return false;}
	virtual const UInt8*		readBytes(UInt32& size){return NULL;}
	virtual Time&				readTime(Time& time) { return time.update(); }
	virtual void				readNull(){}

	virtual bool				readObject(std::string& type,bool& external){return false;}
	virtual bool				readArray(UInt32& size){return false;}
	virtual Type				readItem(std::string& name){return END;}
};


} // namespace Mona
