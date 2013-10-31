/* 
	Copyright 2013 Mona - mathieu.poux[a]gmail.com
 
	This file is a part of Mona.
 
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

#include "Mona/Util.h"
#include "Mona/Exceptions.h"
#include "Mona/String.h"
#include <fstream>

#if !defined(_WIN32)
extern "C" char **environ; // TODO test it on linux!
#endif

using namespace std;

namespace Mona {


MapParameters	Util::_Environment;
mutex			Util::_MutexEnvironment;

map<thread::id, string>	Util::_ThreadNames;
mutex					Util::_MutexThreadNames;


const string& Util::GetThreadName(thread::id id) {
	lock_guard<mutex> lock(_MutexThreadNames);
	return _ThreadNames[id];
}

void Util::SetThreadName(thread::id id, const string& name) {
	lock_guard<mutex> lock(_MutexThreadNames);
	_ThreadNames[id] = name;
}

string& Util::FormatHex(const UInt8* data,UInt32 size,string& result) {
	result.clear();
	for (int i = 0; i < size;++i)
		String::Append(result,Format<UInt8>("%X",data[i]));
	return result;
}

string& Util::FormatHex2(const UInt8* data, UInt32 size, string& result) {
	result.clear();
	for (int i = 0; i < size; ++i)
		String::Append(result, Format<UInt8>("\\x%X", data[i]));
	return result;
}

UInt8 Util::Get7BitValueSize(UInt64 value) {
	UInt64 limit = 0x80;
	UInt8 result=1;
	while(value>=limit) {
		limit<<=7;
		++result;
	}
	return result;
}

// environment variables (TODO test on service windows!!)
const MapParameters& Util::Environment() {
	lock_guard<mutex> lock(_MutexEnvironment);
	if (_Environment.count() > 0)
		return _Environment;
	char *s = *environ;
	for (int i = 0; s = *(environ + i); ++i) {
		const char* temp = strchr(s, '=');
		string name;
		string value;
		if (temp) {
			name.assign(s, temp - s);
			value.assign(temp + 1);
		} else
			name.assign(s);
		_Environment.setString(name, value);
	}
	return _Environment;
}


bool Util::UnpackUrl(Exception& ex, const string& url, string& path, MapParameters& properties) {
	SocketAddress address;
	string file;
	return UnpackUrl(ex, url, address, path, file, properties);
}

bool Util::UnpackUrl(Exception& ex, const string& url, string& path, string& file, MapParameters& properties) {
	SocketAddress address;
	return UnpackUrl(ex, url, address, path, file, properties);
}

bool Util::UnpackUrl(Exception& ex, const string& url, SocketAddress& address, string& path, MapParameters& properties) {
	string file;
	return UnpackUrl(ex, url, address, path, file, properties);
}
// TODO check unitest
bool Util::UnpackUrl(Exception& ex,const string& url, SocketAddress& address, string& path, string& file, MapParameters& properties) {
	
	auto it = url.begin();
	auto end = url.end();
	while (it != end) {
		if (isspace(*it)) {
			ex.set(Exception::FORMATTING, "URL ", url, " malformed, space character");
			return false;
		}
		if (*it == '/' || *it == '\\') // no address!
			break;
		if (*it == ':') {
			// protocol
			while (it != end && (*it == '/' || *it == '\\'))
				++it;
			if (it == end)
				break;
			++it;
			if (it == end) // no address, no path, just "scheme://"
				return true;
			auto itEnd(it);
			while (itEnd != end && (*it == '/' || *it == '\\'))
				++itEnd;
			address.set(ex, string(it, itEnd));
			break;
		}
		++it;
	}
	if (ex || it == end)
		return !ex;

	// Normalize path => replace // by / and \ by / AND remove the last '/'
	path.assign(it,end);
	it = path.begin();
	auto itFile = it;
	end = path.end();
	bool hasFile(false);
	string query;
	while (it != end) {
		if (isspace(*it)) {
			ex.set(Exception::FORMATTING, "URL ", url, " malformed, space character");
			return false;
		}
		if (*it == '?') {
			// query now!
			// file?
			if (hasFile)
				file.assign(itFile, it);
			// query
			UnpackQuery(string(++it, end),properties);
			// trunk the path
			if (hasFile)
				path.erase(--itFile, end);
			else
				path.erase(--it, end);
			return true;
		}
		if (*it == '/' || *it == '\\') {
			++it;
			while (it != end && (*it == '/' || *it == '\\')) 
				path.erase(it++); // erase multiple slashes
			if (it == end) {
				hasFile = false;
				// remove the last /
				path.erase(--it);
				break;
			}
			itFile = it;
		}
		if (*it == '.')
			hasFile = true;
		++it;
	}


	// file?
	if (hasFile) {
		file.assign(itFile, end);
		path.erase(--itFile, end); // trunk the path if file (no query here)
	}
	return true;
}
// TODO check unitest
void Util::UnpackQuery(const string& query, MapParameters& properties) {
	
	string myQuery(query);

	string::iterator it = myQuery.begin();
	auto end = myQuery.end();
	while (it != end) {

		// name
		string name;
		auto itEnd(it);
		while (itEnd != end && *itEnd != '=' && *itEnd != '&') {
			if (*itEnd == '+')
				*itEnd = ' ';
			++itEnd;
		};
		name.assign(it, itEnd);
		it = itEnd;
		string value;
		if (it!=end && *it != '&') { // if it's '='
			++it;
			// value
			auto itEnd(it);
			while (itEnd != end && *itEnd != '&') {
				if (*itEnd == '+')
					*itEnd = ' ';
				++itEnd;
			};
			value.assign(it, itEnd);
			it = itEnd;
		}
		properties.setString(DecodeURI(name), DecodeURI(value));
	}
}

// TODO check unitest
string& Util::DecodeURI(string& uri) {
	auto it = uri.begin();
	auto end = uri.end();
	while (it != end) {
		char c = *it++;
		if (c != '%' || it == end)
			continue;
		auto itDecode(it);
		char hi = *it++;
		if (it == end)
			return uri;
		char lo = *it++;
		if (hi >= '0' && hi <= '9')
			c = hi - '0';
		else if (hi >= 'A' && hi <= 'F')
			c = hi - 'A' + 10;
		else if (hi >= 'a' && hi <= 'f')
			c = hi - 'a' + 10;
		else
			return uri; // syntax error
		c *= 16;
		if (lo >= '0' && lo <= '9')
			c += lo - '0';
		else if (lo >= 'A' && lo <= 'F')
			c += lo - 'A' + 10;
		else if (lo >= 'a' && lo <= 'f')
			c += lo - 'a' + 10;
		else
			return uri; // syntax error
		*itDecode = c;
		uri.erase(++itDecode, it);
	}
	return uri;
}


void Util::Dump(const UInt8* in,UInt32 size,Buffer<UInt8>& out,const string& header) {
	UInt32 len = 0;
	UInt32 i = 0;
	UInt32 c = 0;
	UInt8 b;
	out.resize((UInt32)ceil((double)size / 16) * 67 + (header.empty() ? 0 : (header.size() + 2)));

	if(!header.empty()) {
		out[len++] = '\t';
		c = header.size();
		memcpy(&out[len],header.c_str(),c);
		len += c;
		out[len++] = '\n';
	}

	while (i<size) {
		c = 0;
		out[len++] = '\t';
		while ( (c < 16) && (i+c < size) ) {
			b = in[i+c];
			sprintf((char*)&out[len],out.size()-len,"%X%X ",b>>4, b & 0x0f );
			len += 3;
			++c;
		}
		while (c++ < 16) {
			memcpy((char*)&out[len],"   \0",4);
			len += 3;
		}
		out[len++] = ' ';
		c = 0;
		while ( (c < 16) && (i+c < size) ) {
			b = in[i+c];
			if (b > 31)
				out[len++] = b;
			else
				out[len++] = '.';
			++c;
		}
		while (c++ < 16)
			out[len++] = ' ';
		i += 16;
		out[len++] = '\n';
	}
}



bool Util::ReadIniFile(Exception& ex,const string& path,MapParameters& parameters) {
	ifstream istr(path, ios::in | ios::binary);
	if (!istr.good()) {
		ex.set(Exception::FILE, "Impossible to open ", path, " file");
		return false;
	}
	string section;
	auto eof = char_traits<char>::eof();
	while (!istr.eof()) {
		int c = istr.get();
		while (c != eof && isspace(c))
			c = istr.get();
		if (c == eof)
			return true;
		if (c == ';') {
			while (c != eof && c != '\n')
				c = istr.get();
		} else if (c == '[') {
			section.clear();
			c = istr.get();
			while (c != eof && isblank(c))
				c = istr.get();
			while (c != eof && c != ']' && c != '\n') {
				section += (char)c;
				c = istr.get();
			}
			String::Trim(section, String::TRIM_RIGHT);
		} else {
			string key;
			while (c != eof && isblank(c))
				c = istr.get();
			while (c != eof && c != '=' && c != '\n') {
				key += (char)c;
				c = istr.get();
			}
			string value;
			if (c == '=') {
				c = istr.get();
				while (c != eof && isblank(c))
					c = istr.get();
				while (c != eof && c != '\n') {
					value += (char)c;
					c = istr.get();
				}
			}
			string fullKey = section;
			if (!fullKey.empty())
				fullKey += '.';
			fullKey.append(String::Trim(key, String::TRIM_RIGHT));
			parameters.setString(fullKey, String::Trim(value, String::TRIM_RIGHT));
		}
	}
	return true;
}


} // namespace Mona
