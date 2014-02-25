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

#include "Mona/HTTPOptionsWriter.h"
#include "Mona/HTTP/HTTP.h"
#include "Mona/Logs.h"

using namespace std;


namespace Mona {

HTTPOptionsWriter::HTTPOptionsWriter() : indexCanBeMethod(false),indexDirectory(true),_option(NIL),timeout(HTTP::DefaultTimeout) {
}

void HTTPOptionsWriter::clear() {
	_option = NIL;
	(bool&)indexDirectory = true;
	((string&)index).clear();
	(UInt32&)timeout = HTTP::DefaultTimeout;
}


void HTTPOptionsWriter::writePropertyName(const string& value) {
	if (value == "index")
		_option = INDEX;
	else if (value == "timeout")
		_option = TIMEOUT;
}

void HTTPOptionsWriter::writeNull() {
	if (_option == INDEX) {
		(bool&)indexDirectory = false;
		((string&)index).clear();
	} else if (_option == TIMEOUT) {
		(UInt32&)timeout = 0;
	} else
		ERROR("Unknown HTTP option");
}

void HTTPOptionsWriter::writeNumber(double value) {
	if (_option == INDEX) {
		(bool&)indexDirectory = value!=0 ? true : false;
		((string&)index).clear();
	} else if (_option == TIMEOUT) {
		(UInt32&)timeout = (UInt32)value/1000;
	} else
		ERROR("Unknown HTTP option");
}

void HTTPOptionsWriter::writeString(const string& value) {
	if (_option == INDEX) {
		(bool&)indexDirectory = false;
		((string&)index).assign(value);
		(bool&)indexCanBeMethod = index.find_last_of('.')==string::npos;
	} else if (_option == TIMEOUT) {
		Date date;
		Exception ex;
		if(date.update(ex,value))
			(UInt32&)timeout = (UInt32)date/1000;
		else
			ERROR("Unvalid HTTP timeout option, ",ex.error());
	} else
		ERROR("Unknown HTTP option");
}

void HTTPOptionsWriter::writeBytes(const UInt8* data, UInt32 size) {
	if (_option == INDEX) {
		(bool&)indexDirectory = false;
		((string&)index).assign((const char*)data,size);
		(bool&)indexCanBeMethod = index.find_last_of('.')==string::npos;
	} else if (_option == TIMEOUT) {
		Exception ex;
		_buffer.assign((const char*)data,size);
		(UInt32&)timeout = String::ToNumber(ex,_buffer,timeout)/1000;
		if (ex)
			ERROR("Unvalid HTTP timeout option, ",ex.error());
	} else
		ERROR("Unknown HTTP option");
}


void HTTPOptionsWriter::writeBoolean(bool value) {
	if (_option == INDEX) {
		(bool&)indexDirectory = value;
		((string&)index).clear();
	} else if (_option == TIMEOUT) {
		(UInt32&)timeout = value ? HTTP::DefaultTimeout : 0;
	} else
		ERROR("Unknown HTTP option");
}

void HTTPOptionsWriter::writeDate(const Date& date) {
	if (_option == INDEX) {
		(bool&)indexDirectory = false;
		((string&)index).assign(date.toString(Date::HTTP_FORMAT,_buffer));
		(bool&)indexCanBeMethod = index.find_last_of('.')==string::npos;
	} else if (_option == TIMEOUT) {
		(UInt32&)timeout = (UInt32)date;
	} else
		ERROR("Unknown HTTP option");
}


} // namespace Mona
