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
#include "Mona/PacketWriter.h"
#include "Mona/Time.h"

namespace Mona {

class DataWriterNull;

class DataWriter : public virtual NullableObject {
public:

	virtual void beginObject(const std::string& type="",bool external=false)=0;
	virtual void endObject()=0;

	virtual void writePropertyName(const std::string& value)=0;

	virtual void beginArray(UInt32 size)=0;
	virtual void endArray()=0;

	virtual void writeDate(const Time& date)=0;
	virtual void writeNumber(double value)=0;
	virtual void writeString(const std::string& value)=0;
	virtual void writeBoolean(bool value)=0;
	virtual void writeNull()=0;
	virtual void writeBytes(const UInt8* data,UInt32 size)=0;


	virtual void beginObjectArray(UInt32 size) { beginArray(size); beginObject(); }

	virtual void beginMap(UInt32 size, bool weakKeys = false) { beginObject(); }
	virtual void endMap() { endObject(); }

	virtual bool repeat(UInt32 reference) { return false; }
	virtual void clear();

	UInt32		 lastReference() { return _lastReference; }

	void		 writeNullProperty(const std::string& name);
	void		 writeDateProperty(const std::string& name,const Time& date);
	void		 writeNumberProperty(const std::string& name,double value);
	void		 writeBooleanProperty(const std::string& name,bool value);
	void		 writeStringProperty(const std::string& name,const std::string& value);
		
	PacketWriter packet;

    static DataWriterNull Null;
protected:
	DataWriter(const PoolBuffers& poolBuffers): packet(poolBuffers),_lastReference(0){}
	DataWriter(): NullableObject(true),_lastReference(0){} // Null

	UInt32					_lastReference;

};


class DataWriterNull : public DataWriter {
public:
	DataWriterNull() : NullableObject(true),DataWriter() {}

private:
    void beginObject(const std::string& type="",bool external=false){}
    void endObject(){}

    void writePropertyName(const std::string& value){}

    void beginArray(UInt32 size){}
    void endArray(){}

    void writeDate(const Time& date){}
    void writeNumber(double value){}
    void writeString(const std::string& value){}
    void writeBoolean(bool value){}
    void writeNull(){}
    void writeBytes(const UInt8* data,UInt32 size){}
};




} // namespace Mona
