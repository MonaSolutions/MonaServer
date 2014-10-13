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

#include "Mona/XMLRPCWriter.h"
#include "Mona/Logs.h"


using namespace std;


namespace Mona {

#define RESPONSE_HEADER		"<?xml version=\"1.0\"?><methodResponse><params>"
#define RESPONSE_FOOTER		"</params></methodResponse>"


XMLRPCWriter::XMLRPCWriter(const PoolBuffers& poolBuffers) : DataWriter(poolBuffers) {
	packet.write(EXPAND(RESPONSE_HEADER)).write(EXPAND(RESPONSE_FOOTER));
}

void XMLRPCWriter::clear(UInt32 size) {
	_layers.clear();
	DataWriter::clear(size);
	packet.write(EXPAND(RESPONSE_HEADER)).write(EXPAND(RESPONSE_FOOTER));
}

UInt64 XMLRPCWriter::beginObject(const char* type) {
	start(OBJECT);

	if (type)
		packet.write(EXPAND("<value><struct type=\"")).write(type).write("\">");
	else
		packet.write(EXPAND("<value><struct>"));
	return 0;
}

void XMLRPCWriter::writePropertyName(const char* value) {
	start();
	packet.write(EXPAND("<member><name>")).write(value).write(EXPAND("</name>"));
}

void XMLRPCWriter::endObject() {
	packet.write(EXPAND("</struct></value>"));
	end(true);
}

UInt64 XMLRPCWriter::beginArray(UInt32 size) {
	start(ARRAY);
	packet.write(EXPAND("<value><array size=\""));
	String::Append(packet, size).write("\"><data>");
	return 0;
}

void XMLRPCWriter::endArray() {
	packet.write(EXPAND("</data></array></value>"));
	end(true);
}

void XMLRPCWriter::writeNumber(double value) {
	if (round(value) == value && value <= numeric_limits<int>::max() && value >= numeric_limits<int>::min())
		return writeInt((int)value);
	String::Append(beginValue("double"), value);
	endValue("double");
}


UInt64 XMLRPCWriter::writeDate(const Date& date) {
	string buffer;
	date.toString(Date::ISO8601_FRAC_FORMAT,buffer); 
	beginValue("dateTime.iso8601").write(buffer.data(), buffer.size());
	endValue("dateTime.iso8601");
	return 0;
}

PacketWriter& XMLRPCWriter::beginValue(const char* tag) {
	start();
	packet.write(EXPAND("<value>"));
	if (tag)
		packet.write("<").write(tag).write(">");
	return packet;
}

void XMLRPCWriter::endValue(const char* tag) {
	if (tag)
		packet.write("</").write(tag).write(">");
	packet.write(EXPAND("</value>"));
	end();
}

void XMLRPCWriter::start(Type type) {

	// Write RESPONSE_HEADER
	if (_layers.empty())
		packet.clear(packet.size() - sizeof(RESPONSE_FOOTER) + 1); // remove RESPONSE_FOOTER

	if (_layers.empty())
		packet.write(EXPAND("<param>"));

	if (type)
		_layers.push_back(type==OBJECT);
}

void XMLRPCWriter::end(bool isContainer) {

	if (isContainer) {
		if (_layers.empty()) {
			ERROR("XML-RPC container already closed")
			return;
		}
		_layers.pop_back();
	}

	if (_layers.empty()) {
		packet.write(EXPAND("</param>"));
		packet.write(EXPAND(RESPONSE_FOOTER));
	} else if (_layers.back()) // is member object
		packet.write(EXPAND("</member>"));
}


} // namespace Mona
