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

namespace Mona {

template<class MapType>
class MapWriter : public DataWriter, virtual Object {
public:
	MapWriter() : _size(0) {
		_it = _map.end();
	}
	void beginObject(const std::string& type = "", bool external = false) {}
	void endObject() {}

	void writePropertyName(const std::string& value) { _it = _map.emplace(value,"").first; }

	void beginArray(UInt32 size) {}
	void endArray(){}

	void writeDate(const Date& date) { set(date.toString(Date::SORTABLE_FORMAT,_buffer)); }
	void writeNumber(double value) { set(String::Format(_buffer, _buffer)); }
	void writeString(const std::string& value) { set(value); }
	void writeBoolean(bool value) { set( value ? "true" : "false");}
	void writeNull() { set("null"); }
	void writeBytes(const UInt8* data, UInt32 size) { set((const char*)data, size); }

	UInt32 size() const { return _size; }
	UInt32 count() const { return _map.size(); }
	typename MapType::const_iterator operator[](const std::string& key) { return _map.find(key); }

	typename MapType::const_iterator begin() { return _map.begin(); }
	typename MapType::const_iterator end() { return _map.end(); }

	void   clear() { _map.clear(); _size = 0; }
private:
	template <typename ...Args>
	void set(Args&&... args) {
		if (_it == _map.end())
			_map.emplace(std::piecewise_construct, std::forward_as_tuple(args ...), std::forward_as_tuple(""));
		else {
			_it->second.assign(args ...);
			_size += _it->second.size();;
			_it = _map.end();
		}
	}

	MapType						_map;
	typename MapType::iterator	_it;
	std::string					_buffer;
	UInt32						_size;
};



} // namespace Mona
