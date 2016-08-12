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

#include "Test.h"
#include "Mona/Util.h"

using namespace Mona;

using namespace std;



bool TestEncode(const char* data,UInt32 size, const char* result) {
	static string value;
	return Util::ToBase64((UInt8*)data, size, value) == result;
}

bool TestDecode(string data, const char* result, UInt32 size) {
	return Util::FromBase64(data) && memcmp(data.c_str(),result,size)==0;
}


ADD_TEST(UtilTest, UnpackQuery) {
	string value;
	MapParameters properties;
	CHECK(Util::UnpackQuery("name1=value1&name2=value2", properties).count()==2)
	DEBUG_CHECK(properties.getString("name1", value) && value == "value1");
	DEBUG_CHECK(properties.getString("name2", value) && value == "value2");
	properties.clear();


	string test("name1=one%20space&name2=%22one+double quotes%22&name3=percent:%25&name4=%27simple quotes%27");
	CHECK(Util::UnpackQuery(test, properties).count()==4); // test "count" + DecodeUrI
	DEBUG_CHECK(properties.getString("name1", value) && value == "one space");
	DEBUG_CHECK(properties.getString("name2", value) && value == "\"one double quotes\"");
	DEBUG_CHECK(properties.getString("name3", value) && value == "percent:%");
	DEBUG_CHECK(properties.getString("name4", value) && value == "'simple quotes'");
	CHECK(Util::UnpackQuery("longquery://test;:/one%*$^/fin=value~", properties).count()==5)
	DEBUG_CHECK(properties.getString("longquery://test;:/one%*$^/fin", value) && value == "value~");

	bool next(true);
	Util::ForEachParameter forEach([&next](const string& name, const char* value) { return next; });
	CHECK(Util::UnpackQuery(test.c_str(), forEach) == 4); // test "string::pos" + DecodeUrI
	CHECK(Util::UnpackQuery("name1=value1&name2=value2", 12, forEach) == 1);
	next = false;
	CHECK(Util::UnpackQuery(test, forEach) == 1);

}


ADD_TEST(UtilTest, UnpackUrlPerf) {
	string address;
	string path;
	string query;
	CHECK(Util::UnpackUrl("rtmp://127.0.0.1:1234/path/file.txt?name1=value1&name2=value2", address, path,query)!=string::npos)
}


ADD_TEST(UtilTest, UnpackUrl) {
	string address;
	string path;
	string query;
	string file;

	CHECK(Util::UnpackUrl("/path",path,query)!=string::npos && path=="/path");
	CHECK(Util::UnpackUrl("/",path,query)==string::npos && path=="");
	CHECK(Util::UnpackUrl("/.",path,query)==string::npos && path=="");
	CHECK(Util::UnpackUrl("/..",path,query)==string::npos && path=="");
	CHECK(Util::UnpackUrl("/~",path,query)!=string::npos && path=="/~");
	CHECK(Util::UnpackUrl("/path/.",path,query)==string::npos && path=="/path");
	CHECK(Util::UnpackUrl("/path/..",path,query)==string::npos && path=="");
	CHECK(Util::UnpackUrl("/path/~",path,query)!=string::npos && path=="/path/~");
	CHECK(Util::UnpackUrl("/path/./sub/",path,query)==string::npos && path=="/path/sub");
	CHECK(Util::UnpackUrl("/path/../sub/",path,query)==string::npos && path=="/sub");
	CHECK(Util::UnpackUrl("/path/~/sub/",path,query)==string::npos && path=="/path/~/sub");
	CHECK(Util::UnpackUrl("//path//sub//.",path,query)==string::npos && path=="/path/sub");
	CHECK(Util::UnpackUrl("//path//sub//..",path,query)==string::npos && path=="/path");
	CHECK(Util::UnpackUrl("//path//sub//~",path,query)!=string::npos && path=="/path/sub/~");

	CHECK(Util::UnpackUrl("rtmp://",path,query)==string::npos);
	CHECK(Util::UnpackUrl("rtmp://127.0.0.1", address, path, query)==string::npos)
	CHECK(Util::UnpackUrl("rtmp://127.0.0.1:1234/", address, path, query)==string::npos)
	CHECK(Util::UnpackUrl("rtmp://127.0.0.1:1234/file.txt?", address, path,query)!=string::npos)
	CHECK(Util::UnpackUrl("rtmp://127.0.0.1:1234/file.txt?name1=value1&name2=value2", address, path, query)!=string::npos)
	CHECK(Util::UnpackUrl("rtmp://127.0.0.1:1234//path/file.txt?name1=value1&name2=value2", address, path, query)!=string::npos)

	DEBUG_CHECK(query == "name1=value1&name2=value2");
	DEBUG_CHECK(path=="/path/file.txt");
	DEBUG_CHECK(address=="127.0.0.1:1234");
}


ADD_TEST(UtilTest, Base64) {
	CHECK(TestEncode("\00\01\02\03\04\05", 6,"AAECAwQF"));
	CHECK(TestEncode("\00\01\02\03",4, "AAECAw=="));
	CHECK(TestEncode("ABCDEF", 6,"QUJDREVG"));

	CHECK(TestDecode("AAECAwQF", "\00\01\02\03\04\05", 6));
	CHECK(TestDecode("AAECAw==", "\00\01\02\03", 4));
	CHECK(TestDecode("QUJDREVG", "ABCDEF", 6));
	CHECK(TestDecode("QUJ\r\nDRE\r\nVG", "ABCDEF", 6));
	CHECK(!TestDecode("QUJD#REVG", "ABCDEF", 6));

	static string Message("The quick brown fox jumped over the lazy dog.");
	static string Result;
	Util::ToBase64((const UInt8*)Message.c_str(), Message.size(), Result);
	CHECK(Util::FromBase64(Result) && Result==Message);
	CHECK(Result==Message);


	UInt8 data[255];
	for (UInt8 i = 0; i < 255; ++i)
		data[i] = i;
	Util::ToBase64(data, sizeof(data), Result);
	CHECK(Util::FromBase64(Result));
	CHECK(memcmp(Result.data(), data, sizeof(data)) == 0);
}


ADD_TEST(UtilTest, FormatHex) {
	Buffer buffer;
	Util::FormatHex((const UInt8*)"\00\01\02\03\04\05", 6, buffer,Util::HEX_CPP);
	CHECK(memcmp(buffer.data(),"\\x00\\x01\\x02\\x03\\x04\\x05",buffer.size())==0)

	Util::FormatHex((const UInt8*)"\00\01\02\03\04\05", 6, buffer);
	CHECK(memcmp(buffer.data(),"000102030405",buffer.size())==0)


	Util::UnformatHex(buffer);
	CHECK(memcmp(buffer.data(), "\00\01\02\03\04\05", buffer.size()) == 0)

	buffer.resize(64, false);
	memcpy(buffer.data(), "36393CB1428ECC178FE88D37094D0B3D34B95C0E985177E45336997EBEAB58CD", 64);
	Util::UnformatHex(buffer);
	CHECK(buffer.size() == 32 && memcmp(buffer.data(), "\x36\x39\x3C\xB1\x42\x8E\xCC\x17\x8F\xE8\x8D\x37\x09\x4D\x0B\x3D\x34\xB9\x5C\x0E\x98\x51\x77\xE4\x53\x36\x99\x7E\xBE\xAB\x58\xCD", buffer.size()) == 0)

	string strBuffer("000102030405");
	Util::UnformatHex(strBuffer);
	CHECK(memcmp(strBuffer.data(), "\x00\x01\x02\x03\x04\x05", 6)==0);
}

