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
#include "Poco/URI.h"
#include "Poco/HexBinaryEncoder.h"
#include "Poco/FileStream.h"
#include <sstream>
#include "math.h"

using namespace std;
using namespace Poco;


namespace Mona {

NullInputStream		Util::NullInputStream;
NullOutputStream	Util::NullOutputStream;


string Util::FormatHex(const UInt8* data,UInt32 size) {
	ostringstream oss;
	HexBinaryEncoder(oss).write((const char*)data,size);
	return oss.str();
}

string Util::FormatHex2(const UInt8* data,UInt32 size) {
	ostringstream oss;
	HexBinaryEncoder hex(oss);
	UInt32 i;
	for(i=0;i<size;++i) {
		oss << "\\x";
		hex << data[i];
	}
	return oss.str();
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

bool Util::UnpackUrl(Exception& ex,const string& url, SocketAddress& address, string& path, string& file, MapParameters& properties) {
	URI uri;
	try {
		uri = url;
	} catch (...) {
		ex.set(Exception::FORMATTING,"Unpack url ", url, " impossible");
		return false;
	}
	uri.normalize();
	path = uri.getPath();
	// normalize path "/like/that"
	size_t found = path.rfind('/');
	while(found!= string::npos && found==(path.size()-1)) {
		path.erase(found);
		found = path.rfind('/');
	}
	// file?
	size_t punctFound = path.rfind('.');
	if(punctFound!=string::npos && punctFound>found) {
		if(found==string::npos) {
			file=path;
			path.clear();
		} else {
			file=path.substr(found+1);
			path.erase(found);
		}
	}
	address.set(ex,uri.getHost(),uri.getPort());
	if (ex)
		return false;
	UnpackQuery(uri.getRawQuery(),properties);
	return true;
}

void Util::UnpackQuery(const string& query, MapParameters& properties) {
	istringstream istr(query);
	static const int eof = char_traits<char>::eof();

	int ch = istr.get();
	while (ch != eof) {
		string name;
		string value;
		while (ch != eof && ch != '=' && ch != '&') {
			if (ch == '+') ch = ' ';
			name += (char) ch;
			ch = istr.get();
		}
		if (ch == '=') {
			ch = istr.get();
			while (ch != eof && ch != '&')
			{
				if (ch == '+') ch = ' ';
				value += (char) ch;
				ch = istr.get();
			}
		}
		string decodedName;
		string decodedValue;
		URI::decode(name, decodedName);
		URI::decode(value, decodedValue);
		properties.setString(decodedName,decodedValue);
		if (ch == '&') ch = istr.get();
	}
}


void Util::Dump(const UInt8* in,UInt32 size,vector<UInt8>& out,const string& header) {
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
			strcpy((char*)&out[len],"   ");
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
	FileInputStream istr(path, ios::in);
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




unsigned Util::ProcessorCount() {

#if defined(_WIN32)
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;

#elif defined(POCO_OS_FAMILY_BSD)
	unsigned count;
	std::size_t size = sizeof(count);
	if (sysctlbyname("hw.ncpu", &count, &size, 0, 0))
		return 1;
	return count;

#elif POCO_OS == POCO_OS_HPUX
	return pthread_num_processors_np();

#elif defined(_SC_NPROCESSORS_ONLN)
	int count = sysconf(_SC_NPROCESSORS_ONLN);
	if (count <= 0) count = 1;
	return static_cast<int>(count);

#elif defined(POCO_OS_FAMILY_VMS)

#pragma pointer_size save
#pragma pointer_size 32

	Poco::UInt32 count(1);
	unsigned short length;

	ILE3 items[2];
	items[0].ile3$w_code = SYI$_ACTIVECPU_CNT;
	items[0].ile3$w_length = sizeof(count);
	items[0].ile3$ps_bufaddr = &count;
	items[0].ile3$ps_retlen_addr = &length;
	items[1].ile3$w_code = 0;
	items[1].ile3$w_length = 0;

	sys$getsyiw(0, 0, 0, items, 0, 0, 0);
	return count;
#pragma pointer_size restore

#else
	return 1;
#endif
}


} // namespace Mona
