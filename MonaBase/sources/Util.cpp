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
#include "Mona/Timezone.h"
#include <fstream>
#include <vector>

#if defined(_WIN32)
	#include <windows.h>
#else
	#include <unistd.h>
	#if defined(__APPLE__)
		#include <pthread.h>
	#elif defined(_OS_BSD)
		#include <pthread_np.h>
	#else
		#include <sys/prctl.h> // for thread name
	#endif
	#include <sys/syscall.h>
	extern "C" char **environ;
#endif

using namespace std;

namespace Mona {


mutex					Util::_MutexEnvironment;
MapParameters			Util::_Environment;

recursive_mutex			Util::_MutexThreadNames;
map<THREAD_ID, string>	Util::_ThreadNames;

// KEEP the following lines in this exact order (to be built correctly, and after _Environment variable)
map<Int64,Timezone::Transition> Timezone::_Transitions;
map<Int64,Timezone::Transition> Timezone::_LocalTransitions;
Timezone::TransitionRule		Timezone::_StartDST;
Timezone::TransitionRule		Timezone::_EndDST;
Timezone						Timezone::_Timezone; // to guarantee that it will be build after _Environment

const char* Util::_URICharReserved("%<>{}|\\\"^`#?\x7F");

const char Util::_B64Table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char Util::_ReverseB64Table[128] = {
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
	64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
	64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
};

THREAD_ID Util::CurrentThreadId() {
#ifdef _WIN32
	return GetCurrentThreadId();
#elif _OS_BSD
	return pthread_self();
#else
	return syscall(SYS_gettid);
#endif
}

void SetCurrentThreadDebugName(const char* name) {
	#if defined(_DEBUG)
	#if defined(_WIN32)
		typedef struct tagTHREADNAME_INFO {
			DWORD dwType; // Must be 0x1000.
			LPCSTR szName; // Pointer to name (in user addr space).
			DWORD dwThreadID; // Thread ID (-1=caller thread).
			DWORD dwFlags; // Reserved for future use, must be zero.
		} THREADNAME_INFO;

		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = name;
		info.dwThreadID = GetCurrentThreadId();
		info.dwFlags = 0;

		__try {
			RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
		} __except (EXCEPTION_EXECUTE_HANDLER) {
		}

	#elif defined(_OS_BSD)
		pthread_set_name_np(pthread_self(), name);
	#else
		prctl(PR_SET_NAME, name, 0, 0, 0);
	#endif
	#endif
}

void Util::SetCurrentThreadName(const char* name) {
	SetCurrentThreadDebugName(name);
	THREAD_ID id(CurrentThreadId());
	lock_guard<recursive_mutex> lock(_MutexThreadNames);
	_ThreadNames[id].assign(name);
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
const Parameters& Util::Environment() {
	lock_guard<mutex> lock(_MutexEnvironment);
	if (_Environment.count() > 0)
		return _Environment;
	const char* line(*environ);
	for (UInt32 i = 0; (line = *(environ + i)); ++i) {
		const char* value = strchr(line, '=');
		if (value) {
			string name(line,(value++)-line);
			_Environment.setString(name, value);
		} else
			_Environment.setString(line, NULL);
	}
	return _Environment;
}


size_t Util::UnpackUrl(const char* url, string& address, string& path, string& query) {
	
	path.clear();
	query.clear();

	const char* it = url;

	bool isFile(true);
	vector<size_t> slashs;

	// Get address
	while (*it) {
		if (*it == '/' || *it == '\\') { // no address, just path
			isFile = false;
			break;
		}
		if (*it == ':') {
			++it;
			while (*it && (*it == '/' || *it == '\\'))
				++it;
			if (!*it) // no address, no path, just "scheme://"
				return string::npos;
			const char* itEnd(it);
			while (*itEnd && *itEnd != '/' && *itEnd != '\\')
				++itEnd;
			address.assign(it, itEnd);
			url = it = itEnd; // on slash after address
			isFile = false;
			break;
		}
		++it;
	}


	// Normalize path if was not starting with a first slash
	if (it != url) {
		slashs.emplace_back(0);
		path.assign("/").append(url, it - url);
	}

	// Normalize path => replace // by / and \ by / AND remove the last '/'
    while (*it) {
		// Extract query part
        if (*it == '?') {
			query.assign(++it);
			// remove last slashes
			while (!path.empty() && (path.back() == '/' || path.back() == '\\'))
				path.resize(path.size() - 1);
			break;
		}
		// Add slash
        if (*it == '/' || *it == '\\') {
            ++it;
           while (*it == '/' || *it == '\\')
               ++it;
			isFile = false;

			if (*it == '.') {
				++it;
				if (*it == '.') {
					// ..
					if (slashs.empty()) {
						path.clear();
					} else {
						path.resize(slashs.back());
						slashs.pop_back();
					}	
					while (*it == '.')
						++it;
					continue;
				}
				while (*it == '/' || *it == '\\')
					++it;
			}

			if (*it) {
				slashs.emplace_back(path.size());
				path += '/'; // We don't add the last slash
			}
			continue;
		}
		// Add current character
		if (*it == '+')
			path += ' ';
		else if (*it == '%') {
			DecodeURI(it, path);
			if (*it)
				path += *it;
		} else
			path += *it;
		++it;
		if (!isFile)
			isFile = true;
	}
	return isFile ? (slashs.back()+1) : string::npos;
}

Parameters& Util::UnpackQuery(const char* query, size_t count, Parameters& parameters) {
	ForEachParameter forEach([&parameters](const string& key, const char* value) {
		parameters.setString(key, value);
		return true;
	});
	UnpackQuery(query, count, forEach);
	return parameters;
}

size_t Util::UnpackQuery(const char* query, size_t count, const ForEachParameter& forEach) {
	const char* it(query);
	size_t countPairs(0);
	string name;
	string value;
	while (count && *it) {

		// name
		name.clear();
		while (count && *it && *it != '=' && *it != '&') {
			if (*it == '+')
				name += ' ';
			else if (*it == '%') {
				DecodeURI(it, name);
				if (count && *it)
					name += *it;
			} else
				name += *it;
			++it; --count;
			
		};
		value.clear();
		bool hasValue(false);
		if (count && *it && *it != '&') { // if it's '='
			// value
			hasValue = true;
			++it; count--; // skip '='
			while (count && *it && *it != '&') {
				if (*it == '+')
					value += ' ';
				else if (*it == '%') {
					DecodeURI(it, value);
					continue;
				} else
					value += *it;
					
				++it; --count;
			};
		}
		if (count && *it) { // if it's '&'
			++it; --count;
		}
		if (!forEach(name, hasValue ? value.c_str() : NULL))
			return string::npos;
		++countPairs;
	}
	return countPairs;
}

void Util::Dump(const UInt8* data,UInt32 size,Buffer& buffer) {
	UInt8 b;
	UInt32 c(0);
	buffer.resize((UInt32)ceil((double)size / 16) * 67,false);

	const UInt8* end(data+size);
	UInt8*		 out(buffer.data());

	while (data<end) {
		c = 0;
		*out++ = '\t';
		while ( (c < 16) && (data < end) ) {
			b = *data++;
            snprintf((char*)out,4,"%X%X ",b>>4, b & 0x0f );
			out += 3;
			++c;
		}
		data -= c;
		while (c++ < 16) {
			memcpy((char*)out,"   \0",4);
			out += 3;
		}
		*out++ = ' ';
		c = 0;
		while ( (c < 16) && (data < end) ) {
			b = *data++;
			if (b > 31)
				*out++ = b;
			else
				*out++ = '.';
			++c;
		}
		while (c++ < 16)
			*out++ = ' ';
		*out++ = '\n';
	}
}



bool Util::ReadIniFile(Exception& ex,const string& path,Parameters& parameters) {
	ifstream ifile(path, ios::in | ios::binary | ios::ate);
	if (!ifile.good()) {
		ex.set(Exception::FILE, "Impossible to open ", path, " file");
		return false;
	}
	UInt32 size = (UInt32)ifile.tellg();
	if (size == 0)
		return true;
	vector<char> buffer(size);
	ifile.seekg(0);
	ifile.read(buffer.data(), size);
	UInt32 i(0);
	string section;
	while (i<size) {
		char c(0);
		do {
			c = buffer[i++];
		} while (isspace(c) && i < size);
		if (i==size)
			return true;
		if (c == ';') {
			while (c != '\n' && i<size)
				c = buffer[i++];
		} else if (c == '[') {
			section.clear();
			do {
				c = buffer[i++];
			} while (isblank(c) && i < size);
			do {
				section += c;
				if (i == size)
					break;
				c = buffer[i++];
			} while (c != ']' && c != '\n');
		} else {
			string key;
			while (i < size && c != '=' && c != '\n') {
				key += c;
				c = buffer[i++];
			}
			string value;
			if (i < size && c == '=') {
				do {
					c = buffer[i++];
				} while (isblank(c) && i < size);
				do {
					value += c;
					if (i == size)
						break;
					c = buffer[i++];
				} while (c != '\n');
			}
			string fullKey(String::Trim(section, String::TRIM_RIGHT));
			if (!fullKey.empty())
				fullKey += '.';
			fullKey.append(String::Trim(key, String::TRIM_RIGHT));
			parameters.setString(fullKey, String::Trim(value, String::TRIM_RIGHT));
		}
	}
	return true;
}


} // namespace Mona
