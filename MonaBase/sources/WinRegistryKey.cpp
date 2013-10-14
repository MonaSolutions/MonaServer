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

#include "Mona/WinRegistryKey.h"
#include "Mona/Exception.h"
#if defined(POCO_WIN32_UTF8)
#include "Poco/UnicodeConverter.h"
#endif

#if !defined(_WIN32_WCE)

using namespace std;
using namespace Poco;

namespace Mona {

namespace {
	class AutoHandle {
	public:
		AutoHandle(HMODULE h) : _h(h) {}
		~AutoHandle() { FreeLibrary(_h); }

		HMODULE handle() { return _h; }
	private:
		HMODULE _h;
	};
}

WinRegistryKey::WinRegistryKey(const string& key, bool readOnly, REGSAM extraSam) :
	_hKey(0),
	_readOnly(readOnly),
	_extraSam(extraSam) {
		string::size_type pos = key.find('\\');
		if (pos != string::npos) {
			string rootKey = key.substr(0, pos);
			_hRootKey = handleFor(rootKey);
			_subKey = key.substr(pos + 1);
		} else
			throw Exception(Exception::REGISTRY, key, "not a valid registry key");
}

WinRegistryKey::WinRegistryKey(HKEY hRootKey, const std::string& subKey, bool readOnly, REGSAM extraSam) :
	_hRootKey(hRootKey),
	_subKey(subKey),
	_hKey(0),
	_readOnly(readOnly),
	_extraSam(extraSam) {
}


void WinRegistryKey::setString(const string& name, const string& value) {
	open();
#if defined(POCO_WIN32_UTF8)
	wstring uname;
	UnicodeConverter::toUTF16(name, uname);
	wstring uvalue;
	UnicodeConverter::toUTF16(value, uvalue);
	if (RegSetValueExW(_hKey, uname.c_str(), 0, REG_SZ, (CONST BYTE*) uvalue.c_str(), (DWORD)(uvalue.size() + 1)*sizeof(wchar_t)) != ERROR_SUCCESS)
		throw Exception(Exception::REGISTRY, "Failed to set registry value ", key(name));
#else
	if (RegSetValueExA(_hKey, name.c_str(), 0, REG_SZ, (CONST BYTE*) value.c_str(), (DWORD) value.size() + 1) != ERROR_SUCCESS)
		throw Exception(Exception::REGISTRY, "Failed to set registry value ", key(name));
#endif
}


string WinRegistryKey::getString(const string& name) {
	open();
	DWORD type;
	DWORD size;
#if defined(POCO_WIN32_UTF8)
	wstring uname;
	UnicodeConverter::toUTF16(name, uname);
	if (RegQueryValueExW(_hKey, uname.c_str(), NULL, &type, NULL, &size) != ERROR_SUCCESS || type != REG_SZ && type != REG_EXPAND_SZ)
		Exception(Exception::REGISTRY, "Key ",key(name)," not found");
	if (size > 0) {
		DWORD len = size / 2;
		wchar_t* buffer = new wchar_t[len + 1];
		RegQueryValueExW(_hKey, uname.c_str(), NULL, NULL, (BYTE*)buffer, &size);
		buffer[len] = 0;
		wstring uresult(buffer);
		delete[] buffer;
		string result;
		UnicodeConverter::toUTF8(uresult, result);
		return result;
	}
#else
	if (RegQueryValueExA(_hKey, name.c_str(), NULL, &type, NULL, &size) != ERROR_SUCCESS || type != REG_SZ && type != REG_EXPAND_SZ)
		throw NotFoundException(key(name));
	if (size > 0)
	{
		char* buffer = new char[size + 1];
		RegQueryValueExA(_hKey, name.c_str(), NULL, NULL, (BYTE*) buffer, &size);
		buffer[size] = 0;
		string result(buffer);
		delete [] buffer;
		return result;
	}
#endif
	return string();
}


void WinRegistryKey::setStringExpand(const string& name, const string& value) {
	open();
#if defined(POCO_WIN32_UTF8)
	wstring uname;
	UnicodeConverter::toUTF16(name, uname);
	wstring uvalue;
	UnicodeConverter::toUTF16(value, uvalue);
	if (RegSetValueExW(_hKey, uname.c_str(), 0, REG_EXPAND_SZ, (CONST BYTE*) uvalue.c_str(), (DWORD)(uvalue.size() + 1)*sizeof(wchar_t)) != ERROR_SUCCESS)
		throw Exception(Exception::REGISTRY, "Failed to set registry value ", key(name));
#else
	if (RegSetValueExA(_hKey, name.c_str(), 0, REG_EXPAND_SZ, (CONST BYTE*) value.c_str(), (DWORD) value.size() + 1) != ERROR_SUCCESS)
		throw Exception(Exception::REGISTRY, "Failed to set registry value ", key(name)); 
#endif
}


string WinRegistryKey::getStringExpand(const string& name) {
	open();
	DWORD type;
	DWORD size;
#if defined(POCO_WIN32_UTF8)
	wstring uname;
	UnicodeConverter::toUTF16(name, uname);
	if (RegQueryValueExW(_hKey, uname.c_str(), NULL, &type, NULL, &size) != ERROR_SUCCESS || type != REG_SZ && type != REG_EXPAND_SZ)
		throw Exception(Exception::REGISTRY,"Key ",key(name)," not found");
	if (size > 0) {
		DWORD len = size / 2;
		wchar_t* buffer = new wchar_t[len + 1];
		RegQueryValueExW(_hKey, uname.c_str(), NULL, NULL, (BYTE*)buffer, &size);
		buffer[len] = 0;
		wchar_t temp;
		DWORD expSize = ExpandEnvironmentStringsW(buffer, &temp, 1);
		wchar_t* expBuffer = new wchar_t[expSize];
		ExpandEnvironmentStringsW(buffer, expBuffer, expSize);
		string result;
		UnicodeConverter::toUTF8(expBuffer, result);
		delete[] buffer;
		delete[] expBuffer;
		return result;
	}
#else
	if (RegQueryValueExA(_hKey, name.c_str(), NULL, &type, NULL, &size) != ERROR_SUCCESS || type != REG_SZ && type != REG_EXPAND_SZ)
		throw NotFoundException(key(name));
	if (size > 0)
	{
		char* buffer = new char[size + 1];
		RegQueryValueExA(_hKey, name.c_str(), NULL, NULL, (BYTE*) buffer, &size);
		buffer[size] = 0;
		char temp;
		DWORD expSize = ExpandEnvironmentStringsA(buffer, &temp, 1);	
		char* expBuffer = new char[expSize];
		ExpandEnvironmentStringsA(buffer, expBuffer, expSize);
		string result(expBuffer);
		delete [] buffer;
		delete [] expBuffer;
		return result;
	}
#endif
	return string();
}


void WinRegistryKey::setInt(const string& name, int value) {
	open();
	DWORD data = value;
#if defined(POCO_WIN32_UTF8)
	wstring uname;
	UnicodeConverter::toUTF16(name, uname);
	if (RegSetValueExW(_hKey, uname.c_str(), 0, REG_DWORD, (CONST BYTE*) &data, sizeof(data)) != ERROR_SUCCESS)
		throw Exception(Exception::REGISTRY, "Failed to set registry value ", key(name));
#else
	if (RegSetValueExA(_hKey, name.c_str(), 0, REG_DWORD, (CONST BYTE*) &data, sizeof(data)) != ERROR_SUCCESS)
		throw Exception(Exception::REGISTRY, "Failed to set registry value ", key(name));
#endif
}


int WinRegistryKey::getInt(const string& name) {
	open();
	DWORD type;
	DWORD data;
	DWORD size = sizeof(data);
#if defined(POCO_WIN32_UTF8)
	wstring uname;
	UnicodeConverter::toUTF16(name, uname);
	if (RegQueryValueExW(_hKey, uname.c_str(), NULL, &type, (BYTE*)&data, &size) != ERROR_SUCCESS || type != REG_DWORD)
		throw Exception(Exception::REGISTRY, "Key ", key(name), " not found");
#else
	if (RegQueryValueExA(_hKey, name.c_str(), NULL, &type, (BYTE*) &data, &size) != ERROR_SUCCESS || type != REG_DWORD)
		throw NotFoundException(key(name));
#endif
	return data;
}


void WinRegistryKey::deleteValue(const string& name) {
	open();
#if defined(POCO_WIN32_UTF8)
	wstring uname;
	UnicodeConverter::toUTF16(name, uname);
	if (RegDeleteValueW(_hKey, uname.c_str()) != ERROR_SUCCESS)
		throw Exception(Exception::REGISTRY, "Key ", key(name), " not found");
#else
	if (RegDeleteValueA(_hKey, name.c_str()) != ERROR_SUCCESS)
		throw NotFoundException(key(name));
#endif
}


void WinRegistryKey::deleteKey() {
	Keys keys;
	subKeys(keys);
	close();
	for (Keys::iterator it = keys.begin(); it != keys.end(); ++it) {
		string subKey(_subKey);
		subKey += "\\";
		subKey += *it;
		WinRegistryKey subRegKey(_hRootKey, subKey);
		subRegKey.deleteKey();
	}

	// NOTE: RegDeleteKeyEx is only available on Windows XP 64-bit SP3, Windows Vista or later.
	// We cannot call it directly as this would prevent the code running on Windows XP 32-bit.
	// Therefore, if we need to call RegDeleteKeyEx (_extraSam != 0) we need to check for
	// its existence in ADVAPI32.DLL and call it indirectly.
#if defined(POCO_WIN32_UTF8)
	wstring usubKey;
	UnicodeConverter::toUTF16(_subKey, usubKey);

	typedef LONG(WINAPI *RegDeleteKeyExWFunc)(HKEY hKey, const wchar_t* lpSubKey, REGSAM samDesired, DWORD Reserved);
	if (_extraSam != 0) {
		AutoHandle advAPI32(LoadLibraryW(L"ADVAPI32.DLL"));
		if (advAPI32.handle()) {
			RegDeleteKeyExWFunc pRegDeleteKeyExW = reinterpret_cast<RegDeleteKeyExWFunc>(GetProcAddress(advAPI32.handle(), "RegDeleteKeyExW"));
			if (pRegDeleteKeyExW) {
				if ((*pRegDeleteKeyExW)(_hRootKey, usubKey.c_str(), _extraSam, 0) != ERROR_SUCCESS)
					throw Exception(Exception::REGISTRY, "Key ", key(), " not found");
				return;
			}
		}
	}
	if (RegDeleteKeyW(_hRootKey, usubKey.c_str()) != ERROR_SUCCESS)
		throw Exception(Exception::REGISTRY, "Key ", key(), " not found");
#else
	typedef LONG (WINAPI *RegDeleteKeyExAFunc)(HKEY hKey, const char* lpSubKey, REGSAM samDesired, DWORD Reserved);
	if (_extraSam != 0)
	{
		AutoHandle advAPI32(LoadLibraryA("ADVAPI32.DLL"));
		if (advAPI32.handle())
		{
			RegDeleteKeyExAFunc pRegDeleteKeyExA = reinterpret_cast<RegDeleteKeyExAFunc>(GetProcAddress(advAPI32.handle() , "RegDeleteKeyExA"));
			if (pRegDeleteKeyExA)
			{
				if ((*pRegDeleteKeyExA)(_hRootKey, _subKey.c_str(), _extraSam, 0) != ERROR_SUCCESS)
					throw Exception(Exception::REGISTRY, "Key ", key(), " not found");
				return;
			}
		}
	}
	if (RegDeleteKey(_hRootKey, _subKey.c_str()) != ERROR_SUCCESS)
		throw Exception(Exception::REGISTRY, "Key ", key(), " not found");
#endif
}


bool WinRegistryKey::exists() {
	HKEY hKey;
#if defined(POCO_WIN32_UTF8)
	wstring usubKey;
	UnicodeConverter::toUTF16(_subKey, usubKey);
	if (RegOpenKeyExW(_hRootKey, usubKey.c_str(), 0, KEY_READ | _extraSam, &hKey) == ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return true;
	}
#else
	if (RegOpenKeyExA(_hRootKey, _subKey.c_str(), 0, KEY_READ | _extraSam, &hKey) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return true;
	}
#endif
	return false;
}


WinRegistryKey::Type WinRegistryKey::type(const string& name) {
	open();
	DWORD type = REG_NONE;
	DWORD size;
#if defined(POCO_WIN32_UTF8)
	wstring uname;
	UnicodeConverter::toUTF16(name, uname);
	if (RegQueryValueExW(_hKey, uname.c_str(), NULL, &type, NULL, &size) != ERROR_SUCCESS)
		throw Exception(Exception::REGISTRY, "Key ", key(name), " not found");
#else
	if (RegQueryValueExA(_hKey, name.c_str(), NULL, &type, NULL, &size) != ERROR_SUCCESS)
		throw NotFoundException(key(name));
#endif
	if (type != REG_SZ && type != REG_EXPAND_SZ && type != REG_DWORD)
		throw Exception(Exception::REGISTRY, key(name), ", type not supported");

	Type aType = (Type)type;
	return aType;
}


bool WinRegistryKey::exists(const string& name) {
	bool exists = false;
	HKEY hKey;
#if defined(POCO_WIN32_UTF8)
	wstring usubKey;
	UnicodeConverter::toUTF16(_subKey, usubKey);
	if (RegOpenKeyExW(_hRootKey, usubKey.c_str(), 0, KEY_READ | _extraSam, &hKey) == ERROR_SUCCESS) {
		wstring uname;
		UnicodeConverter::toUTF16(name, uname);
		exists = RegQueryValueExW(hKey, uname.c_str(), NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
		RegCloseKey(hKey);
	}
#else
	if (RegOpenKeyExA(_hRootKey, _subKey.c_str(), 0, KEY_READ | _extraSam, &hKey) == ERROR_SUCCESS)
	{
		exists = RegQueryValueExA(hKey, name.c_str(), NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
		RegCloseKey(hKey);
	}
#endif
	return exists;
}


void WinRegistryKey::open() {
	if (!_hKey) {
#if defined(POCO_WIN32_UTF8)
		wstring usubKey;
		UnicodeConverter::toUTF16(_subKey, usubKey);
		if (_readOnly) {
			if (RegOpenKeyExW(_hRootKey, usubKey.c_str(), 0, KEY_READ | _extraSam, &_hKey) != ERROR_SUCCESS)
				throw Exception(Exception::REGISTRY,"Cannot open registry ", key());
		} else {
			if (RegCreateKeyExW(_hRootKey, usubKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE | _extraSam, NULL, &_hKey, NULL) != ERROR_SUCCESS)
				throw Exception(Exception::REGISTRY, "Cannot open registry ", key());
		}
#else
		if (_readOnly) {
			if (RegOpenKeyExA(_hRootKey, _subKey.c_str(), 0, KEY_READ | _extraSam, &_hKey) != ERROR_SUCCESS)
				throw Exception(Exception::REGISTRY,"Cannot open registry ", key());
		}
		else
		{
			if (RegCreateKeyExA(_hRootKey, _subKey.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE | _extraSam, NULL, &_hKey, NULL) != ERROR_SUCCESS)
				throw Exception(Exception::REGISTRY,"Cannot open registry ", key());
		}
#endif
	}
}


void WinRegistryKey::close() {
	if (_hKey) {
		RegCloseKey(_hKey);
		_hKey = 0;
	}
}


string WinRegistryKey::key() const {
	string result;
	if (_hRootKey == HKEY_CLASSES_ROOT)
		result = "HKEY_CLASSES_ROOT";
	else if (_hRootKey == HKEY_CURRENT_CONFIG)
		result = "HKEY_CURRENT_CONFIG";
	else if (_hRootKey == HKEY_CURRENT_USER)
		result = "HKEY_CURRENT_USER";
	else if (_hRootKey == HKEY_LOCAL_MACHINE)
		result = "HKEY_LOCAL_MACHINE";
	else if (_hRootKey == HKEY_USERS)
		result = "HKEY_USERS";
	else if (_hRootKey == HKEY_PERFORMANCE_DATA)
		result = "HKEY_PERFORMANCE_DATA";
	else
		result = "(UNKNOWN)";
	result += '\\';
	result += _subKey;
	return result;
}


string WinRegistryKey::key(const string& valueName) const {
	string result = key();
	if (!valueName.empty()) {
		result += '\\';
		result += valueName;
	}
	return result;
}


HKEY WinRegistryKey::handleFor(const string& rootKey) {
	if (rootKey == "HKEY_CLASSES_ROOT")
		return HKEY_CLASSES_ROOT;
	else if (rootKey == "HKEY_CURRENT_CONFIG")
		return HKEY_CURRENT_CONFIG;
	else if (rootKey == "HKEY_CURRENT_USER")
		return HKEY_CURRENT_USER;
	else if (rootKey == "HKEY_LOCAL_MACHINE")
		return HKEY_LOCAL_MACHINE;
	else if (rootKey == "HKEY_USERS")
		return HKEY_USERS;
	else if (rootKey == "HKEY_PERFORMANCE_DATA")
		return HKEY_PERFORMANCE_DATA;
	else
		throw Exception(Exception::REGISTRY,"Not a valid root key ", rootKey);
}


void WinRegistryKey::subKeys(WinRegistryKey::Keys& keys) {
	open();

	DWORD subKeyCount = 0;
	DWORD valueCount = 0;

	if (RegQueryInfoKey(_hKey, NULL, NULL, NULL, &subKeyCount, NULL, NULL, &valueCount, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
		return;

#if defined(POCO_WIN32_UTF8)
	wchar_t buf[256];
	DWORD bufSize = sizeof(buf) / sizeof(wchar_t);
	for (DWORD i = 0; i < subKeyCount; ++i) {
		if (RegEnumKeyExW(_hKey, i, buf, &bufSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
			wstring uname(buf);
			string name;
			UnicodeConverter::toUTF8(uname, name);
			keys.push_back(name);
		}
		bufSize = sizeof(buf) / sizeof(wchar_t);
	}
#else
	char buf[256];
	DWORD bufSize = sizeof(buf);
	for (DWORD i = 0; i< subKeyCount; ++i)
	{
		if (RegEnumKeyExA(_hKey, i, buf, &bufSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			string name(buf);
			keys.push_back(name);
		}
		bufSize = sizeof(buf);
	}
#endif
}


void WinRegistryKey::values(WinRegistryKey::Values& vals) {
	open();

	DWORD valueCount = 0;

	if (RegQueryInfoKey(_hKey, NULL, NULL, NULL, NULL, NULL, NULL, &valueCount, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
		return;

#if defined(POCO_WIN32_UTF8)
	wchar_t buf[256];
	DWORD bufSize = sizeof(buf) / sizeof(wchar_t);
	for (DWORD i = 0; i < valueCount; ++i) {
		if (RegEnumValueW(_hKey, i, buf, &bufSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
			wstring uname(buf);
			string name;
			UnicodeConverter::toUTF8(uname, name);
			vals.push_back(name);
		}
		bufSize = sizeof(buf) / sizeof(wchar_t);
	}
#else
	char buf[256];
	DWORD bufSize = sizeof(buf);
	for (DWORD i = 0; i< valueCount; ++i)
	{
		if (RegEnumValueA(_hKey, i, buf, &bufSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			string name(buf);
			vals.push_back(name);
		}
		bufSize = sizeof(buf);
	}
#endif
}


} // namespace Mona


#endif // !defined(_WIN32_WCE)
