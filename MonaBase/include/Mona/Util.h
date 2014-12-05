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
#include "Mona/MapParameters.h"
#include "Mona/Time.h"
#include "Mona/Exceptions.h"
#include "Mona/Buffer.h"
#include "math.h"
#include <limits>
#include <mutex>
#include <thread>

namespace Mona {


class Util : virtual Static {
public:
	
	static UInt8 Get7BitValueSize(UInt32 value) { return Get7BitValueSize((UInt64)value); }
	static UInt8 Get7BitValueSize(UInt64 value);

	static THREAD_ID	CurrentThreadId();
	static void			SetCurrentThreadName(const char* name);
	template <typename BufferType>
	static BufferType&	GetThreadName(THREAD_ID id, BufferType& buffer, bool append = false) {
		if (!append)
			buffer.resize(0);
		std::lock_guard<std::recursive_mutex> lock(_MutexThreadNames);
		return String::Append(buffer,_ThreadNames[id]);
	}

	static void Dump(const UInt8* data, UInt32 size, Buffer& buffer) { std::string header; Dump(data, size, buffer, header); }
	static void Dump(const UInt8* data, UInt32 size, Buffer& buffer, const std::string& header);

	/// \brief Unpack url in path and query
	/// \param url Url to unpack
	/// \param path Part of the url between host address and '?'
	/// \param query Part of the url after '?' (if present)
	/// \return The position of the file in path (std::npos for a directory)
	static std::size_t UnpackUrl(const std::string& url, std::string& path, std::string& query) {std::string address; return UnpackUrl(url, address, path, query);}
	static std::size_t UnpackUrl(const char* url, std::string& path, std::string& query) {std::string address; return UnpackUrl(url, address, path, query);}
	static std::size_t UnpackUrl(const std::string& url, std::string& address, std::string& path, std::string& query) { return UnpackUrl(url.c_str(), address, path, query); }
	static std::size_t UnpackUrl(const char*, std::string& address, std::string& path, std::string& query);
	
	typedef std::function<bool(const std::string&, const char*)> ForEachParameter;

	static Parameters& UnpackQuery(const std::string& query, Parameters& parameters) { return UnpackQuery(query.c_str(), parameters); }
	static Parameters& UnpackQuery(const char* query, Parameters& parameters);
	static std::size_t UnpackQuery(const std::string& query, const ForEachParameter& forEach) { return UnpackQuery(query.c_str(), forEach); }
	static std::size_t UnpackQuery(const char* query, const ForEachParameter& forEach);


	static bool ReadIniFile(Exception& ex, const std::string& path, Parameters& parameters);

	static unsigned ProcessorCount() { unsigned result(std::thread::hardware_concurrency());  return result > 0 ? result : 1; }
	static const Parameters& Environment();


	template <typename BufferType>
	static const char* DecodeURI(const char* value,BufferType& out) {
		if (!*value || *value != '%')
			return value; // nothing, end!
		if (!*++value) {
			String::Append(out,'%');
			return value; // syntax error
		}
		char hi = *value;
		if (!*++value) {
			String::Append(out,'%',hi);
			return value; // syntax error
		}
		char lo = *value++;
		char c;
		if (hi >= '0' && hi <= '9')
			c = hi - '0';
		else if (hi >= 'A' && hi <= 'F')
			c = hi - 'A' + 10;
		else if (hi >= 'a' && hi <= 'f')
			c = hi - 'a' + 10;
		else {
			String::Append(out,'%',hi,lo);
			return value; // syntax error
		}
		c *= 16;
		if (lo >= '0' && lo <= '9')
			c += lo - '0';
		else if (lo >= 'A' && lo <= 'F')
			c += lo - 'A' + 10;
		else if (lo >= 'a' && lo <= 'f')
			c += lo - 'a' + 10;
		else {
			String::Append(out,'%',hi,lo);
			return value; // syntax error
		}
	
		// Decoded! Assign next caracter & return the caracter decoded
		String::Append(out,c);
		return value;
	}

	template <typename BufferType>
	static BufferType&	EncodeURI(const char* in, BufferType& out) { return EncodeURI(in, strlen(in), out); }

	template <typename BufferType>
	static BufferType&	EncodeURI(const char* in, UInt32 size, BufferType& out) {
		const char* end(in + size);
		while (in<end) {
			char c = *in++;
			if (isxml(c))
				Buffer::Append(out, &c, 1);
			else if (c <= 0x20 || c > 0x7E || strchr(_URICharReserved,c))
				String::Append(out, '%', Format<UInt8>("%2X", (UInt8)c));
			else
				Buffer::Append(out, &c, 1);
		}
		return out;
	}

	template<typename Type>
	static Type Random() {
		static UInt32 x = (UInt32)Time::Now();
		static UInt32 y = 362436069, z = 521288629, w = 88675123;
		UInt32 t = x ^ (x << 11);
		x = y; y = z; z = w;
		return (w = w ^ (w >> 19) ^ (t ^ (t >> 8))) % std::numeric_limits<Type>::max();
	}
	static void Random(UInt8* data, UInt32 size) {for (UInt32 i = 0; i < size; ++i) data[i] = Random<UInt8>();}

	
	template <typename BufferType>
	static BufferType& ToBase64(const UInt8* data, UInt32 size, BufferType& buffer,bool append=false) {
		UInt32 accumulator(buffer.size()),bits(0);

		if (!append)
			accumulator = 0;
		buffer.resize(accumulator+(UInt32)ceil(size/3.0)*4,append);

		char*		current = (char*)buffer.data();
		if (!current) // to expect null writer 
			return buffer;
		const char*	end(current+buffer.size());
		current += accumulator;

		const UInt8* endData = data + size;
		
		accumulator = 0;
		while(data<endData) {
			accumulator = (accumulator << 8) | (*data++ & 0xFFu);
			bits += 8;
			while (bits >= 6) {
				bits -= 6;
				*current++ = _B64Table[(accumulator >> bits) & 0x3Fu];
			}
		}
		if (bits > 0) { // Any trailing bits that are missing.
			accumulator <<= 6 - bits;
			*current++ = _B64Table[accumulator & 0x3Fu];
		}
		while (current<end) // padding with '='
			*current++ = '=';
		return buffer;
	}


	template <typename BufferType>
	static bool FromBase64(BufferType& buffer) { return FromBase64((const UInt8*)buffer.data(), buffer.size(), buffer); }

	template <typename BufferType>
	static bool FromBase64(const UInt8* data, UInt32 size,BufferType& buffer,bool append=false) {
		UInt32 bits(0), accumulator(0), oldSize(buffer.size());
		if (!append)
			oldSize = 0;

		UInt8*		 out((UInt8*)buffer.data());
		if (!out) // to expect null writer 
			return false;
		out += oldSize;
		const UInt8* end(data+size);

		size = 0;
	
		while(data<end) {
			const int c = *data++;
			if (isspace(c) || c == '=')
				continue;

			if ((c > 127) || (c < 0) || (_ReverseB64Table[c] > 63)) {
				// reset the oldSize
				buffer.resize(oldSize,append);
				return false;
			}
		
			accumulator = (accumulator << 6) | _ReverseB64Table[c];
			bits += 6;
			if (bits >= 8) {
				bits -= 8;
				size++;
				*out++ = ((accumulator >> bits) & 0xFFu);
			}
		}
		buffer.resize(oldSize+size,append);
		return true;
	}

	enum HexOption {
		HEX_CPP=1,
		HEX_TRIM_LEFT=2,
		HEX_APPEND=4,
		HEX_UPPER_CASE=8
	};

	template <typename BufferType>
	static BufferType&	FormatHex(const UInt8* data, UInt32 size, BufferType& buffer, UInt8 options=0) {
		if (!buffer.data()) // to expect null writer 
			return buffer;

		const UInt8* end(data+size);
		bool skipLeft(false);
		if (options&HEX_TRIM_LEFT) {
			while(data<end) {
				if (((*data) >> 4)>0)
					break;
				if (((*data) & 0x0F) > 0) {
					skipLeft = true;
					break;
				}
				++data;
			}
		}

		UInt32 oldSize(options&HEX_APPEND ? buffer.size() : 0);
		buffer.resize((end-data) * ((options&HEX_CPP) ? 4 : 2) - (skipLeft ? 1 : 0) + oldSize, options&HEX_APPEND);
		UInt8* out((UInt8*)buffer.data() + oldSize);
	
		UInt8 ref(options&HEX_UPPER_CASE ? '7' : 'W');

		UInt8 value;
		while(data<end) {
			if (options&HEX_CPP) {
				*out++ = '\\';
				*out++ = 'x';
			}
			value = (*data) >> 4;
			if (!skipLeft)
				*out++ = (value>9 ? (value + ref) : '0' + value);
			else
				skipLeft = false;
			value = (*data) & 0x0F;
			*out++ = (value > 9 ? (value + ref) : '0' + value);
			++data;
		}
		return buffer;
	}

	template <typename BufferType>
	static BufferType& UnformatHex(BufferType& buffer) {
		UInt32		 size(0);
		UInt8*		 in = (UInt8*)buffer.data();
		if (!in) // to expect null writer 
			return buffer;
		UInt8*		 out = in;
		const UInt8* end = in+buffer.size();

		while(in<end) {
			UInt8 first = toupper(*in++);
			UInt8 second = (in == end) ? '0' : toupper(*in++);
			*out++ = ((first - (first<='9' ? '0' : '7')) << 4) | ((second - (second<='9' ? '0' : '7')) & 0x0F);
			++size;
		}
		buffer.resize(size, false);
		return buffer;
	}


private:

	static MapParameters					_Environment;
	static std::mutex						_MutexEnvironment;
	
	static std::map<THREAD_ID, std::string>	_ThreadNames;
	static std::recursive_mutex				_MutexThreadNames;

	static const char*						_URICharReserved;
	static const char						_B64Table[65];
	static const char						_ReverseB64Table[128];
};


} // namespace Mona
