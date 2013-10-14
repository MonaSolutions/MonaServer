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
*/


#pragma once

#include "Mona/Mona.h"
#include "Mona/MapParameters.h"

namespace Mona {


class IniFileParameters : public MapParameters {
public:
	IniFileParameters(const std::string& path);
	void load(std::istream& istr);
		/// Loads the configuration data from the given stream, which 
		/// must be in initialization file format.
		
	void load(const std::string& path);
		/// Loads the configuration data from the given file, which 
		/// must be in initialization file format.

protected:
	bool getRaw(const std::string& key, std::string& value) const;
	void setRaw(const std::string& key, const std::string& value);
	void enumerate(const std::string& key, Keys& range) const;
	void removeRaw(const std::string& key);
	~IniFileConfiguration();

private:
	void parseLine(std::istream& istr);

	struct ICompare
	{
		bool operator () (const std::string& s1, const std::string& s2) const;
	};
	typedef std::map<std::string, std::string, ICompare> IStringMap;
	

};


} // namespace Mona
