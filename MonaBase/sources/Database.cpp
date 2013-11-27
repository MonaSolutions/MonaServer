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

#include "Mona/Database.h"
#include "Mona/Files.h"
#include "Mona/FileSystem.h"
#include "Mona/Util.h"
#include <fstream>
#include <openssl/evp.h>

using namespace std;

namespace Mona {

void Database::load(Exception& ex, const string& rootPath, DatabaseLoader& loader,bool disableTransaction) {
	flush();
	_rootPath = rootPath;
	_disableTransaction = disableTransaction;
	FileSystem::MakeDirectory(_rootPath);
	if (FileSystem::Exists(_rootPath))
		loadDirectory(ex, _rootPath , "", loader);
	_disableTransaction = false;
}

bool Database::add(Exception& ex, const string& path, const UInt8* value, UInt32 size) {
	if (_disableTransaction)
		return true;
	lock_guard<mutex> lock(_mutex);
	if (!running() && !start(ex,Startable::PRIORITY_LOWEST))
		return false;
	_entries.emplace_back(path,value,size);
	wakeUp();
	return true;
}

bool Database::remove(Exception& ex, const string& path) {
	if (_disableTransaction)
		return true;
	lock_guard<mutex> lock(_mutex);
	if (!running() && !start(ex, Startable::PRIORITY_LOWEST))
		return false;
	_entries.emplace_back(path);
	wakeUp();
	return true;
}


void Database::run(Exception& ex) {

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


void Database::processEntry(Exception& ex,Entry& entry) {
	string directory(_rootPath);
	directory.append(entry.path);
	// remove folder
	Exception exRemove;
	FileSystem::RemoveAll(exRemove, directory);
	if (entry.toRemove) {
		if(exRemove)
			ex.set(exRemove);
		return;
	}

	// add entry
	FileSystem::CreateDirectories(ex,directory);
	if (ex)
		return;

	// compute md5
	UInt8 result[16];
	EVP_Digest(entry.buffer.data(), entry.buffer.size(), result, NULL, EVP_md5(), NULL);
	string name;
	Util::FormatHex(result, sizeof(result), name);

	// write the file
	string file(FileSystem::MakeDirectory(directory));
	file.append(name);
	ofstream ofile(file, ios::out | ios::binary);
	if (!ofile.good()) {
		ex.set(Exception::FILE, "Impossible to write file ", file, " in ", directory);
		return;
	}
	ofile.write((const char*)entry.buffer.data(), entry.buffer.size());
}


bool Database::loadDirectory(Exception& ex, const string& directory, const string& path, DatabaseLoader& loader) {
	Files files(ex, directory);
	if (ex)
		return true;

	bool hasData = false;
	for (const string& file : files) {
		string folder(file);
		FileSystem::MakeDirectory(folder);
		string sub(directory);
		sub.append(folder);
		if (FileSystem::Exists(sub)) {
			/// directory
			hasData = loadDirectory(ex, sub, path + "/" + file, loader);
			continue;
		}

		/// file
		if (file.size() != 32) {
			// just erase the file
			FileSystem::Remove(file);
			continue;
		}

		FileSystem::MakeFile(sub);
		// read the file
		ifstream ifile(sub, ios::in | ios::binary | ios::ate);
		if (!ifile.good()) {
			ex.set(Exception::FILE, "Impossible to read file ", sub);
			return true;
		}
		UInt32 size = (UInt32)ifile.tellg();
		ifile.seekg(0);
		Buffer<char> buffer(size);
		if (size>0)
			ifile.read(buffer.data(), size);
		// compute md5
		UInt8 result[16];
		EVP_Digest(buffer.data(), size, result, NULL, EVP_md5(), NULL);
		string name;
		Util::FormatHex(result, sizeof(result), name);
		// compare with file name
		if (memcmp(name.data(), file.c_str(), name.size()) != 0) {
			// erase this data!
			FileSystem::Remove(file);
			continue;
		}

		hasData = true;
		loader.onDataLoading(path, buffer.data(), buffer.size());

	}
	if (!hasData)
		FileSystem::RemoveAll(ex,directory);
	return hasData;
}

} // namespace Mona
