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

#include "Mona/Options.h"

using namespace Poco;
using namespace std;

namespace Mona {


Options::Options() : _pOption(NULL){
}

Options::~Options() {
}

void Options::add(Exception& ex, const Option& option) {
	if (option.fullName().empty()) {
		ex.set(Exception::OPTION,"Invalid option (fullName is empty)");
		return;
	}
	if (option.shortName().empty()) {
		ex.set(Exception::OPTION, "Invalid option (shortName is empty)");
		return;
	}
	auto result = _options.insert(option);
	if (!result.second) {
		ex.set(Exception::OPTION, "Option ",option.fullName(), " (", option.shortName() , ") duplicated");
		return;
	}
}


const Option& Options::get(Exception& ex, const string& name) const {
	auto result = _options.find(Option(name, ""));
	if (result==_options.end())
		ex.set(Exception::OPTION, "Unknown ",name," option");
	return *result;
}

void Options::process(Exception& ex, int argc, char* argv[], const function<void(const string&, const string&)>& handler) {
	_pOption = NULL;
	string name, value;
	set<string> alreadyReaden;
	for (int i = 1; i < argc; ++i) {
		if (process(ex, argv[i], name, value, alreadyReaden) && handler)
			handler(name, value);
	}

	if (_pOption)
		ex.set(Exception::ARGUMENT, _pOption->fullName(), " requires ", _pOption->argumentName());
	for (const Option& option : _options) {
		if (option.required() && alreadyReaden.find(option.fullName()) == alreadyReaden.end())
			ex.set(Exception::OPTION, "Option ", option.fullName(), " required");
	}
}

bool Options::process(Exception& ex, const string& argument, string& name, string& value, set<string>& alreadyReaden) {
	string::const_iterator it = argument.begin();
	string::const_iterator end = argument.end();

	if (!_pOption) {
		if (it == end || (*it != '-' && *it != '/') || ++it == end)
			return false;
		// second -?
		if (*it == '-' && ++it == end)
			return false;

		string::const_iterator itEnd = it;

		while (itEnd != end && *itEnd != ':' && *itEnd != '=')
			++itEnd;

		if (it == itEnd)
			ex.set(Exception::OPTION, "Empty option");

		name.assign(it, itEnd);
		_pOption = &get(ex, name);
		if (alreadyReaden.find(_pOption->fullName()) != alreadyReaden.end() && !_pOption->repeatable()) {
			_pOption = NULL;
			ex.set(Exception::OPTION, "option ", name, " duplicated");
		}
		alreadyReaden.insert(_pOption->fullName());

		if(itEnd!=end)
			++itEnd;

		if (itEnd == end && _pOption->argumentRequired())
			return false;

		it = itEnd;
	}

	if (it == end && _pOption->argumentRequired()) {
		_pOption = NULL;
		ex.set(Exception::ARGUMENT, _pOption->fullName(), " requires ", _pOption->argumentName());
	}

	value.assign(it, end);
	if (_pOption->_handler)
		_pOption->_handler(value);
	_pOption = NULL;
	return true;
}



} // namespace Mona
