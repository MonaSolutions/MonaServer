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
#include "Mona/Option.h"
#include <set>

namespace Mona {

class Options {
public:
	typedef std::set<Option>::const_iterator Iterator;

	Options();
	virtual ~Options();

	void			add(const Option& option);
	void			remove(const std::string& name) { _options.erase(Option(name, "")); }

	const Option&	get(const std::string& name) const;

	Iterator		begin() const { return _options.begin(); }
	Iterator		end() const { return _options.end(); }


	void process(int argc, char* argv[], const std::function<void(const std::string&, const std::string&)>& handler = nullptr);

private:
	bool process(const std::string& argument,std::string& name,std::string& value, std::set<std::string>& alreadyReaden);
	void handleOption(const std::string& name,const std::string& value) {}
	
	std::set<Option>	_options;
	const Option* 		_pOption;
};



} // namespace Mona