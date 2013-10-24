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
#include "Poco/UnbufferedStreamBuf.h"
#include <sstream>

namespace Mona {

class StringBuf : public std::stringbuf {
public:
	const char*  data();
};

inline const char* StringBuf::data() {
	return gptr();
}

class BinaryBuffer : public Poco::UnbufferedStreamBuf {
       
public:
	BinaryBuffer();
	~BinaryBuffer();
	UInt32		size();
	const UInt8*	data();

private:
	std::streampos seekpos(std::streampos sp,std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
	std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
	std::streamsize xsputn(const char_type* s, std::streamsize n);
	std::streamsize xsgetn(char_type* p, std::streamsize count);
#if defined(POCO_OS_FAMILY_WINDOWS)
	std::streamsize _Xsgetn_s(char_type * _Ptr,size_t _Ptr_size, std::streamsize _Count);
#endif

	int_type readFromDevice();
	int_type writeToDevice(char_type);

	StringBuf  _buf;
};

inline const UInt8* BinaryBuffer::data() {
	return (const UInt8*)_buf.data();
}

inline BinaryBuffer::int_type BinaryBuffer::writeToDevice(char_type ch) {
	return _buf.sputc(ch);
}

inline std::streamsize BinaryBuffer::xsputn (const char_type* s, std::streamsize n){
        return _buf.sputn(s,n);
}

inline std::streampos BinaryBuffer::seekpos(std::streampos sp,std::ios_base::openmode which) {
       return _buf.pubseekpos(sp,which);
}

inline std::streampos BinaryBuffer::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which) {
        return _buf.pubseekoff(off,way,which);
}

inline std::streamsize BinaryBuffer::xsgetn(char_type* p,std::streamsize count) {
        return _buf.sgetn(p,count);
}

#if defined(POCO_OS_FAMILY_WINDOWS) && _MSC_VER<1600
inline std::streamsize BinaryBuffer::_Xsgetn_s(char_type * _Ptr,size_t _Ptr_size, std::streamsize _Count) {
        return _buf._Sgetn_s(_Ptr,_Ptr_size,_Count);
}
#endif


class BinaryIOS: public virtual std::ios {
       
public:
        BinaryBuffer* rdbuf();

protected:
        BinaryIOS();
        ~BinaryIOS();

private:
        BinaryBuffer  _buf;
};

inline BinaryBuffer* BinaryIOS::rdbuf() {
        return &_buf;
}



class BinaryStream : public BinaryIOS, public std::iostream {
public:
      BinaryStream();
      ~BinaryStream();

	  UInt32			size();
	  const UInt8*	data();

      void            clear();
	  void            resetReading(UInt32 position);
	  void			  resetWriting(UInt32 position);
	  void			  next(UInt32 count);
      bool            empty();
};

inline UInt32 BinaryStream::size() {
      return rdbuf()->size();
}


inline const UInt8* BinaryStream::data() {
	return rdbuf()->data();
}


inline bool BinaryStream::empty() {
        return size()==0;
}


} // namespace Mona
