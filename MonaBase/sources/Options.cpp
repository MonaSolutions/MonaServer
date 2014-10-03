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

#include "Mona/Options.h"


using namespace std;

namespace Mona {


const Option& Options::get(const string& name) const {
	auto result = _options.find(Option(name.c_str(), ""));
	if (result == _options.end())
		return Option::Null;
	return *result;
}

bool Options::process(Exception& ex, int argc, const char* argv[], const ForEach& forEach) {
	_pOption = NULL;
	string name, value;
	set<string> alreadyReaden;
	for (int i = 1; i < argc; ++i) {
		if (!process(ex, argv[i], name, value, alreadyReaden))
			return false;
		if (forEach)
			forEach(name, value);
	}

	if (_pOption) {
		ex.set(Exception::ARGUMENT, _pOption->fullName(), " requires ", _pOption->argumentName());
		return false;
	}
		
	for (const Option& option : _options) {
		string lowerName(option.fullName());
		if (option.required() && alreadyReaden.find(String::ToLower(lowerName)) == alreadyReaden.end()) {
			ex.set(Exception::OPTION, "Option ", option.fullName(), " required");
			return false;
		}
	}
	return true;
}

bool Options::process(Exception& ex,const char* argument, string& name, string& value, set<string>& alreadyReaden) {

	if (!_pOption) {
		if (*argument==0 || (*argument != '-' && *argument != '/') || *++argument == 0)
			return false;
		// second -?
		if (*argument == '-' && *++argument == 0)
			return false;

		const char* itEnd = argument;

		while (*itEnd && *itEnd != ':' && *itEnd != '=')
			++itEnd;

		if (argument == itEnd) {
			ex.set(Exception::OPTION, "Empty option");
			return false;
		}
			
		name.assign(argument, itEnd);

		auto itOption = _options.find(Option(name.c_str(),""));
		if (itOption == _options.end()) {
			Option shortOption("", name.c_str());
			for (itOption = _options.begin(); itOption != _options.end(); ++itOption) {
				if (*itOption == shortOption)
					break;
			}
			if (itOption == _options.end()) {
				if (!acceptUnknownOption) {
					ex.set(Exception::OPTION, "Unknown ", name, " option");
					return false;
				}
	
				// name is necessary the full name here!
				auto result = _options.emplace(name.c_str(), name.c_str());
				if (!result.second) {
					ex.set(Exception::OPTION, "Option ", name," duplicated with ", result.first->fullName(), " (", result.first->shortName(), ")");
					return false;
				}
				itOption = result.first;
			} else
				name.assign(itOption->fullName());
		}

		_pOption = &*itOption;
		string lowerName(_pOption->fullName());
		if (alreadyReaden.find(String::ToLower(lowerName)) != alreadyReaden.end() && !_pOption->repeatable()) {
			_pOption = NULL;
			ex.set(Exception::OPTION, "Option ", name, " duplicated");
			return false;
		}
		alreadyReaden.insert(lowerName);

		if(*itEnd)
			++itEnd;

		if (*itEnd == 0 && _pOption->argumentRequired())
			return false;

		argument = itEnd;
	}

	if (*argument == 0 && _pOption->argumentRequired()) {
		_pOption = NULL;
		ex.set(Exception::ARGUMENT, _pOption->fullName(), " requires ", _pOption->argumentName());
		return false;
	}

	value.assign(argument);
	bool result(true);
	if (_pOption->_handler)
		result = _pOption->_handler(ex, value);
	_pOption = NULL;
	return result;
}



} // namespace Mona
