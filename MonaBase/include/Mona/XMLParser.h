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
#include "Mona/MapParameters.h"
#include "Mona/PacketWriter.h"
#include "Mona/Exceptions.h"
#include <vector>


namespace Mona {


class XMLParser : public virtual Object {

private:

	struct Tag {
		Tag() : name(NULL), size(0), full(false) {}
		char*		name;
		UInt32		size;
		bool		full;
	};

	//// TO OVERRIDE /////

	virtual bool onStartXMLDocument() { return true; }
	
	virtual bool onXMLInfos(const char* name, Parameters& attributes) { return true; }

	virtual bool onStartXMLElement(const char* name, Parameters& attributes) = 0;
	virtual bool onInnerXMLElement(const char* name, const char* data, UInt32 size) = 0;
	virtual bool onEndXMLElement(const char* name) = 0;

	virtual void onEndXMLDocument(const char* error) {}

	/////////////////////

public:

	class XMLState : virtual NullableObject {
		friend class XMLParser;
	public:
		XMLState() : _current(NULL) {}
		operator bool() const { return _current ? true : false; }
		void clear() { _current = NULL; }
	private:
		bool						_started;
		Exception					_ex;
		const char*					_current;
		std::vector<Tag>			_tags;
	};

	void reset();
	void reset(const XMLState& state);
	void save(XMLState& state);
	bool parse(Exception& ex);

protected:

	XMLParser(const char* data, UInt32 size, const PoolBuffers& poolBuffers) : _started(false),_reseted(false),_current(data),_running(false),_inner(poolBuffers),_buffer(0),_start(data),_end(data+size) {}
	XMLParser(const char* data, UInt32 size) : _started(false),_current(data),_running(false),_reseted(false),_inner(_buffer),_start(data),_end(data+size) {}

private:

	bool		parse();
	const char* parseXMLName(const char* endMarkers, UInt32& size);

	
	MapParameters				_attributes;
	bool						_running;
	bool						_reseted;
	PacketWriter				_inner;
	Buffer						_buffer;

	// state
	bool						_started;
	Exception					_ex;
	const char*					_current;
	std::vector<Tag>			_tags;

	// const
	const char*					_start;
	const char*					_end;
};


} // namespace Mona
