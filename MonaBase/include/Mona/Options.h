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
#include "Mona/Option.h"
#include "Mona/Exceptions.h"
#include <set>

namespace Mona {

class Options : public virtual Object {
public:
	typedef std::set<Option>::const_iterator Iterator;

	Options() : _pOption(NULL) {}

	bool acceptUnknownOption;

	template <typename ...Args>
	Option& add(Exception& ex, const char* fullName, const char* shortName, Args&&... args) {
        if (strlen(fullName)==0) {
			ex.set(Exception::OPTION, "Invalid option (fullName is empty)");
			return Option::Null;
		}
        if (strlen(shortName) == 0) {
			ex.set(Exception::OPTION, "Invalid option (shortName is empty)");
			return Option::Null;
		}
		auto result = _options.emplace(fullName, shortName, args ...);
		if (!result.second) {
			ex.set(Exception::OPTION, "Option ", fullName, " (", shortName, ") duplicated with ", result.first->fullName(), " (", result.first->shortName(), ")");
			return Option::Null;
		}
		return const_cast<Option&>(*result.first);
	}

	void			remove(const std::string& name) { _options.erase(Option(name.c_str(), "")); }
	void			clear() { _options.clear(); }

	const Option&	get(const std::string& name) const;

	Iterator		begin() const { return _options.begin(); }
	Iterator		end() const { return _options.end(); }

	UInt32			count() const { return _options.size(); }
	bool			empty() const { return _options.empty(); }

	typedef std::function<void(const std::string&, const std::string&)> ForEach;
    bool			process(Exception& ex, int argc, const char* argv[], const ForEach& forEach = nullptr);

private:
	bool			process(Exception& ex, const char* argument, std::string& name, std::string& value, std::set<std::string>& alreadyReaden);
	void			handleOption(const std::string& name,const std::string& value) {}
	
	std::set<Option>	_options;
	const Option* 		_pOption;
};



} // namespace Mona
