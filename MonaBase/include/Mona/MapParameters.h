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
#include "Mona/Parameters.h"
#include <map>

namespace Mona {

class MapParameters : public virtual Object, public Parameters {
public:

	typedef std::map<std::string,std::string>::const_iterator const_iterator;

	std::string&		operator[](const std::string& key) { return _map[key]; }
	const_iterator		begin() const { return _map.begin(); }
	const_iterator		end() const { return _map.end(); }

	UInt32				count() const { return _map.size(); }
	void				clear() { _map.clear(); }

private:
	void				iteration(const char* prefix, ForEach& function) const;
	const std::string*  getRaw(const std::string& key) const;
	void				setRaw(const std::string& key, const char* value);

	std::map<std::string,std::string>	_map;
};


} // namespace Mona
