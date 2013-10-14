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
#include "Mona/Parameters.h"
#include <unordered_map>

namespace Mona {


class MapParameters : public IterableParameters<std::unordered_map<std::string, std::string>::const_iterator> {
public:
	MapParameters();
	virtual ~MapParameters();


	Iterator	begin() const;
	Iterator	end() const;

	void		clear();
	
protected:
	bool getRaw(const std::string& key, std::string& value) const;
	void setRaw(const std::string& key, const std::string& value);

private:	
	std::unordered_map<std::string, std::string> _map;
};

inline void MapParameters::clear() {
	_map.clear();
}

inline void MapParameters::setRaw(const std::string& key, const std::string& value) {
	_map[key] = value;
}
inline MapParameters::Iterator MapParameters::begin() const {
	return _map.begin();
}
inline MapParameters::Iterator MapParameters::end() const {
	return _map.end();
}


} // namespace Mona
