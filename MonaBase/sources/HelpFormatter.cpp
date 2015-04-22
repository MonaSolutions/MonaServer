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


#include "Mona/HelpFormatter.h"
#include "Mona/Logs.h"

using namespace std;


namespace Mona {

#define TAB_WIDTH		4
#define WIDTH			78

#if defined(_WIN32)
#define LONG_PREFIX		"/"
#define SHORT_PREFIX	"/"
#else
#define LONG_PREFIX		"--"
#define SHORT_PREFIX	"-"
#endif


int HelpFormatter::calcIndent() const {
	int indent = 0;
	for (const Option& option : _options) {
		int shortLen = option.shortName().length();
		int fullLen = option.fullName().length();
		int n = 0;
#ifdef _OS_UNIX
        n += shortLen + sizeof(SHORT_PREFIX) + 2;
		if (option.takesArgument())
			n += option.argumentName().length() + (option.argumentRequired() ? 0 : 2);
#endif
		n += fullLen + sizeof(LONG_PREFIX) + 1;
		if (option.takesArgument())
			n += 1 + option.argumentName().length() + (option.argumentRequired() ? 0 : 2);
		if (n > indent)
			indent = n;
	}
	return indent;
}

void HelpFormatter::flush(ostream& ostr) const {
	ostr << "usage: " << command;
	if (!usage.empty()) {
		ostr << ' ';
		formatText(ostr, usage, (int)command.length() + 1);
	}
	ostr << '\n';
	if (!header.empty()) {
		formatText(ostr, header, 0);
		ostr << "\n\n";
	}
	formatOptions(ostr);
	if (!footer.empty()) {
		ostr << '\n';
		formatText(ostr, footer, 0);
		ostr << '\n';
	}
}


void HelpFormatter::formatOptions(ostream& ostr) const {
	for(const Option& option : _options) {
		formatOption(ostr,option);
		formatText(ostr, option.description(), _indent);
		ostr << '\n';
	}
}


void HelpFormatter::formatOption(ostream& ostr, const Option& option) const
{
	int shortLen = option.shortName().length();
	int fullLen  = option.fullName().length();

	int n = 0;

#ifdef _OS_UNIX
    ostr << SHORT_PREFIX << option.shortName();
    n += sizeof(SHORT_PREFIX) + option.shortName().length()+2;
	if (option.takesArgument()) {
		if (!option.argumentRequired()) {
			ostr << '[';
			++n;
		}
		ostr << option.argumentName();
		n += option.argumentName().length();
		if (!option.argumentRequired()) {
			ostr << ']';
			++n;
		}
	}

	ostr << ", ";
	n += 2;
#endif

	ostr << LONG_PREFIX << option.fullName();
	n += sizeof(LONG_PREFIX)-1+option.fullName().length();
	if (option.takesArgument()) {
		if (!option.argumentRequired()) {
			ostr << '[';
			++n;
		}
		ostr << '=';
		++n;
		ostr << option.argumentName();
		n += option.argumentName().length();
		if (!option.argumentRequired()) {
			ostr << ']';
			++n;
		}
	}

	while (n < _indent) {
		ostr << ' ';
		++n;
	}
}


void HelpFormatter::formatText(ostream& ostr, const string& text, int indent) const {
	int pos = _indent;
	size_t maxWordLen = WIDTH - indent;
	string word;
	for (const char c: text) {
		if (c == '\n') {
			flushWord(ostr, pos, word, indent);
			ostr << '\n';
			pos = 0;
			while (pos < indent) {
				ostr << ' ';
				++pos;
			}
		} else if (c == '\t') {
			flushWord(ostr, pos, word, indent);
			if (pos < WIDTH) ++pos;
			while (pos < WIDTH && pos % TAB_WIDTH != 0) {
				ostr << ' ';
				++pos;
			}
		} else if (c == ' ') {
			flushWord(ostr, pos, word, indent);
			if (pos < WIDTH) {
				ostr << ' ';
				++pos;
			}
		} else  {
			if (word.length() == maxWordLen)
				flushWord(ostr, pos, word, indent);
			else
				word += c;
		}
	}
	flushWord(ostr, pos, word, indent);
}

void HelpFormatter::flushWord(ostream& ostr, int& pos, string& word, int indent) const {
	if ((pos + word.length()) > WIDTH) {
		ostr << '\n';
		pos = 0;
		while (pos < indent) {
			ostr << ' ';
			++pos;
		}
	}
	ostr << word;
	pos += word.length();
	word.clear();
}


} // namespace Mona
