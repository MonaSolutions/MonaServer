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

#include "Mona/DataWriter.h"

using namespace std;


namespace Mona {

void DataWriter::writeNullProperty(const string& name) {
	writePropertyName(name);
	writeNull();
}
void DataWriter::writeDateProperty(const string& name,const Time& date) {
	writePropertyName(name);
	writeDate(date);
}
void DataWriter::writeNumberProperty(const string& name,double value) {
	writePropertyName(name);
	writeNumber(value);
}
void DataWriter::writeBooleanProperty(const string& name,bool value) {
	writePropertyName(name);
	writeBoolean(value);
}
void DataWriter::writeStringProperty(const string& name,const string& value) {
	writePropertyName(name);
	writeString(value);
}
void DataWriter::clear() {
	_lastReference=0;
	packet.clear();
}



} // namespace Mona
