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

#include "Mona/PersistentData.h"
#if defined(_WIN32)
	#include "windows.h"
#endif
#include "Mona/FileSystem.h"
#include "Mona/Util.h"
#include <fstream>
#include <openssl/evp.h>


using namespace std;

namespace Mona {

void PersistentData::load(Exception& ex, const string& rootPath, const ForEach& forEach, bool disableTransaction) {
	flush();
	_disableTransaction = disableTransaction;
	FileSystem::MakeFile(_rootPath=rootPath);
	loadDirectory(ex,_rootPath , "", forEach);
	_disableTransaction = false;
}

bool PersistentData::add(Exception& ex, const string& path, const UInt8* value, UInt32 size) {
	if (_disableTransaction)
		return true;
	lock_guard<mutex> lock(_mutex);
	if (!running() && !start(ex,Startable::PRIORITY_LOWEST))
		return false;
	_entries.emplace_back(_poolBuffers,path,value,size);
	wakeUp();
	return true;
}

bool PersistentData::remove(Exception& ex, const string& path) {
	if (_disableTransaction)
		return true;
	lock_guard<mutex> lock(_mutex);
	if (!running() && !start(ex, Startable::PRIORITY_LOWEST))
		return false;
	_entries.emplace_back(_poolBuffers,path);
	wakeUp();
	return true;
}


void PersistentData::run(Exception& ex) {

	Entry*	pEntry;

	for (;;) {

		WakeUpType wakeUpType = sleep(60000); // 1 min timeout

		for (;;) {
			
			{
				lock_guard<mutex> lock(_mutex);
				if (_entries.empty()) {
					if (wakeUpType != WAKEUP) { // STOP or TIMEOUT
						stop(); // to set running=false!
						return;
					}
					break;
				}
				pEntry = &_entries.front();
			}

			processEntry(ex,*pEntry);

			lock_guard<mutex> lock(_mutex);
			_entries.pop_front();
			if (ex) {
				stop();
				return;
			}
		}
	}
}


void PersistentData::processEntry(Exception& ex,Entry& entry) {
	string directory(_rootPath);

	// Security => formalize entry.path to avoid possible /../.. issue
	if (entry.path.empty() || (entry.path.front() != '/' && entry.path.front() != '\\'))
		entry.path.insert(0, "/");

	FileSystem::MakeFolder(directory.append(FileSystem::Resolve(entry.path)));
	string name,file;

	if (!entry.toRemove) {

		// add entry
		if (!FileSystem::CreateDirectory(ex, directory, FileSystem::HEAVY)) {
			ex.set(Exception::FILE, "Impossible to create database directory ", directory);
			return;
		}

		// compute md5
		UInt8 result[16];
		EVP_Digest(entry.pBuffer->data(), entry.pBuffer->size(), result, NULL, EVP_md5(), NULL);
		
		Util::FormatHex(result, sizeof(result), name);

		// write the file
		file.assign(directory).append(name);
#if defined(_WIN32)
		wchar_t wFile[_MAX_PATH];
		MultiByteToWideChar(CP_UTF8, 0, file.c_str(), -1, wFile, _MAX_PATH);
		ofstream ofile(wFile, ios::out | ios::binary);
#else
		ofstream ofile(file, ios::out | ios::binary);
#endif
		if (!ofile.good()) {
			ex.set(Exception::FILE, "Impossible to write file ", file);
			return;
		}
		ofile.write((const char*)entry.pBuffer->data(), entry.pBuffer->size());
	}

	// remove possible old value after writing to be safe!
	bool hasData=false;
	FileSystem::ForEach forEach([&ex, name, &file, &hasData](const string& path, UInt16 level){
		// Delete just hex file
		if (FileSystem::IsFolder(path) || FileSystem::GetName(path, file).size() != 32 || file == name) {
			hasData = true;
			return;
		}
		// is MD5?
		for (char c : file) {
			if (!isxdigit(c) || (c > '9' && isupper(c))) {
				hasData = true;
				return;
			}
		}
		if (!FileSystem::Delete(ex, path))
			hasData = true;
	});
	Exception ignore;
	// if directory doesn't exists it's already deleted so ignore ListFiles exception
	FileSystem::ListFiles(ignore, directory, forEach);
	// if remove and no sub value, erase the folder
	if (entry.toRemove && !hasData)
		FileSystem::Delete(ex,directory);
}


bool PersistentData::loadDirectory(Exception& ex, const string& directory, const string& path, const ForEach& forEach) {
	
	bool hasData = false;

	FileSystem::ForEach forEachFile([&ex, this, &hasData, path, &forEach](const string& file, UInt16 level) {
		/// directory

		string name;
		string value;
		FileSystem::GetName(file, name);
		if (FileSystem::IsFolder(file)) {
			if (loadDirectory(ex, file, String::Format(value, path, '/', name), forEach))
				hasData = true;
			return;
		}
	
		/// file
		if (name.size() != 32) {// ignore this file (not create by Database)
			hasData = true;
			return;
		}

		// read the file
		#if defined(_WIN32)
			wchar_t wFile[_MAX_PATH];
			MultiByteToWideChar(CP_UTF8, 0, file.c_str(), -1, wFile, _MAX_PATH);
			ifstream ifile(wFile, ios::in | ios::binary | ios::ate);
		#else
			ifstream ifile(file, ios::in | ios::binary | ios::ate);
		#endif
		if (!ifile.good()) {
			ex.set(Exception::FILE, "Impossible to read file ", file);
			return;
		}
		UInt32 size = (UInt32)ifile.tellg();
		ifile.seekg(0);
		vector<char> buffer(size);
		if (size>0)
			ifile.read(buffer.data(), size);
		// compute md5
		UInt8 result[16];
		EVP_Digest(buffer.data(), size, result, NULL, EVP_md5(), NULL);
		Util::FormatHex(result, sizeof(result), value); // HEX lower case
		// compare with file name
		if (value != name) {
			// erase this data! (bad serialization)
			// is MD5?
			for (char c : name) {
				if (!isxdigit(c) || (c > '9' && isupper(c))) {
					hasData = true;
					return;
				}
			}
			if(!FileSystem::Delete(ex, file))
				hasData = true;
			return;
		}

		hasData = true;
		forEach(path, (const UInt8*)buffer.data(), buffer.size());
	});

	Exception ignore;
	FileSystem::ListFiles(ignore, directory, forEachFile);
	return hasData || !FileSystem::Delete(ex,directory);
}


} // namespace Mona

