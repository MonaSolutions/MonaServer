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

#include "Mona/Util.h"
#include "Mona/Exceptions.h"
#include "Mona/String.h"
#include "Mona/Time.h"
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

static const char B64Table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char ReverseB64Table[128] = {
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
	64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
	64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
};

const string& Util::GetThreadName(thread::id id) {
	lock_guard<mutex> lock(_MutexThreadNames);
	return _ThreadNames[id];
}

void Util::SetThreadName(thread::id id, const string& name) {
	lock_guard<mutex> lock(_MutexThreadNames);
	_ThreadNames[id] = name;
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
			// address
			++it;
			while (it != end && (*it == '/' || *it == '\\'))
				++it;
			if (it == end) {
				 // no address, no path, just "scheme://"
				return true;
			}
			auto itEnd(it);
			string::const_iterator itPort=end;
			while (itEnd != end && *itEnd != '/' && *itEnd != '\\') {
				if (*itEnd == ':')
					itPort = itEnd;
				++itEnd;
			}
			if (itPort == end)
				address.set(ex, string(it, itEnd),0);
			else {
				string host(it, itPort);
				address.set(ex, host, string(++itPort, itEnd));
			}
			it = itEnd;
			break;
		}
		++it;
	}
	if (ex)
		return false;

	file.clear();
	path.clear();

	// Normalize path => replace // by / and \ by / AND remove the last '/'
	path.assign(it,end);
    auto itPath = path.begin();
    auto itFile = itPath;
    auto endPath = path.end();
	bool hasFile(false);
	string query;
    while (itPath != endPath) {
        if (isspace(*itPath)) {
			ex.set(Exception::FORMATTING, "URL ", url, " malformed, space character");
			return false;
		}
        if (*itPath == '?') {
			// query now!
			// file?
			if (hasFile)
                file.assign(itFile, itPath);
			// query
            UnpackQuery(string(++itPath, endPath),properties);
			// trunk the path
			if (hasFile)
                path.erase(--itFile, endPath);
			else
                path.erase(--itPath, endPath);
			return true;
		}
        if (*itPath == '/' || *itPath == '\\') {
            ++itPath;
            while (itPath != endPath && (*itPath == '/' || *itPath == '\\'))
                path.erase(itPath++); // erase multiple slashes
            if (itPath == endPath) {
				hasFile = false;
				// remove the last /
                path.erase(--itPath);
				break;
			}
            itFile = itPath;
		}
        if (*itPath == '.')
			hasFile = true;
        ++itPath;
	}


	// file?
	if (hasFile) {
        file.assign(itFile, endPath);
        path.erase(--itFile, endPath); // trunk the path if file (no query here)
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
			if (itEnd != end) // if it's '&'
				++itEnd;
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
	out.resize((UInt32)ceil((double)size / 16) * 67 + (header.empty() ? 0 : (header.size() + 2)),false);

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
            snprintf((char*)&out[len],out.size()-len,"%X%X ",b>>4, b & 0x0f );
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

UInt8* Util::UnformatHex(UInt8* data,UInt32& size) {
	UInt32 sizeResult = (UInt32)ceil(size/ 2.0);
	int j = 0;
	for (int i = 0; i < sizeResult; ++i) {
		const char first = data[j++];
		const char second = j == size ? '0' : data[j++];
		data[i] = ((first - (isdigit(first) ? '0' : '7')) << 4) | ((second - (isdigit(second) ? '0' : '7')) & 0x0F);
	}
	size = sizeResult;
	return data;
}

string& Util::FormatHex(const UInt8* data, UInt32 size, string& result) {
	result.clear();
	for (int i = 0; i < size; ++i)
		String::Append(result, Format<UInt8>("%.2X", data[i]));
	return result;
}

string& Util::FormatHexCpp(const UInt8* data, UInt32 size, string& result) {
	result.clear();
	for (int i = 0; i < size; ++i)
		String::Append(result, Format<UInt8>("\\x%.2X", data[i]));
	return result;
}


Buffer<UInt8>& Util::ToBase64(const UInt8* data, UInt32 size, Buffer<UInt8>& result) {
	int i = 0;
	int j = 0;
	UInt32 accumulator = 0;
	UInt32 bits = 0;
	result.resize((UInt32)ceil(size/3.0)*4,false);

	for (i = 0; i < size;++i) {
		accumulator = (accumulator << 8) | (data[i] & 0xFFu);
		bits += 8;
		while (bits >= 6) {
			bits -= 6;
			result[j++] = B64Table[(accumulator >> bits) & 0x3Fu];
		}
	}
	if (bits > 0) { // Any trailing bits that are missing.
		accumulator <<= 6 - bits;
		result[j++] = B64Table[accumulator & 0x3Fu];
	}
	while (result.size() > j) // padding with '='
		result[j++] = '=';
	return result;
}

bool Util::FromBase64(const UInt8* data, UInt32 size, Buffer<UInt8>& result) {
	UInt32 bits = 0;
	UInt32 accumulator = 0;
	result.resize(size/4 * 3,false);
	int j = 0;

	for (int i = 0; i < size; ++i) {
		const int c = data[i];
		if (isspace(c) || c == '=')
			continue;

		if ((c > 127) || (c < 0) || (ReverseB64Table[c] > 63))
			return false;
		
		accumulator = (accumulator << 6) | ReverseB64Table[c];
		bits += 6;
		if (bits >= 8) {
			bits -= 8;
			result[j++] = ((accumulator >> bits) & 0xFFu);
		}
	}
	result.resize(j,true);
	return true;
}

void Util::Random(UInt8* data, UInt32 size) {
	for (UInt32 i = 0; i < size; ++i)
		data[i] = Random<UInt8>();
}


} // namespace Mona
