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
#include "Mona/BinaryWriter.h"
#include "Mona/BinaryStream.h"
#include "Mona/Time.h"

namespace Mona {


class DataWriter {
public:
	virtual ~DataWriter(){}

	virtual void beginObject(const std::string& type="",bool external=false)=0;
	virtual void endObject()=0;

	virtual void writePropertyName(const std::string& value)=0;

	virtual void beginArray(Mona::UInt32 size)=0;
	virtual void endArray()=0;

	virtual void writeDate(const Mona::Time& date)=0;
	virtual void writeNumber(double value)=0;
	virtual void writeString(const std::string& value)=0;
	virtual void writeBoolean(bool value)=0;
	virtual void writeNull()=0;
	virtual void writeBytes(const Mona::UInt8* data,Mona::UInt32 size)=0;


	virtual void			beginObjectArray(Mona::UInt32 size);

	virtual void			beginMap(Mona::UInt32 size,bool weakKeys=false);
	virtual void			endMap();

	virtual bool			repeat(Mona::UInt32 reference);
	virtual void			clear();

	Mona::UInt32	lastReference();

	void			writeNullProperty(const std::string& name);
	void			writeDateProperty(const std::string& name,const Mona::Time& date);
	void			writeNumberProperty(const std::string& name,double value);
	void			writeBooleanProperty(const std::string& name,bool value);
	void			writeStringProperty(const std::string& name,const std::string& value);
	
	BinaryWriter			writer;
	BinaryStream			stream;
protected:
	DataWriter():writer(stream),_lastReference(0){}

	Mona::UInt32	_lastReference;
};

inline Mona::UInt32	DataWriter::lastReference() {
	return _lastReference;
}

inline void DataWriter::beginMap(Mona::UInt32 count,bool weakKeys) {
	beginObject();
}
inline void DataWriter::endMap() {
	endObject();
}
inline void DataWriter::beginObjectArray(Mona::UInt32 size) {
	beginArray(size);beginObject();
}
inline bool DataWriter::repeat(Mona::UInt32 reference) {
	return false;
}


inline void	DataWriter::writeNullProperty(const std::string& name) {
	writePropertyName(name);
	writeNull();
}
inline void	DataWriter::writeDateProperty(const std::string& name,const Mona::Time& date) {
	writePropertyName(name);
	writeDate(date);
}
inline void	DataWriter::writeNumberProperty(const std::string& name,double value) {
	writePropertyName(name);
	writeNumber(value);
}
inline void	DataWriter::writeBooleanProperty(const std::string& name,bool value) {
	writePropertyName(name);
	writeBoolean(value);
}
inline void	DataWriter::writeStringProperty(const std::string& name,const std::string& value) {
	writePropertyName(name);
	writeString(value);
}
inline void	DataWriter::clear() {
	_lastReference=0;
	stream.resetWriting(0);
}


class DataWriterNull : public DataWriter {
public:
	DataWriterNull() {stream.setstate(std::ios_base::eofbit);}

private:
	void beginObject(const std::string& type="",bool external=false){}
	void endObject(){}

	void writePropertyName(const std::string& value){}

	void beginArray(Mona::UInt32 size){}
	void endArray(){}

	void writeDate(const Mona::Time& date){}
	void writeNumber(double value){}
	void writeString(const std::string& value){}
	void writeBoolean(bool value){}
	void writeNull(){}
	void writeBytes(const Mona::UInt8* data,Mona::UInt32 size){}
};




} // namespace Mona
