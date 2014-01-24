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

bool Database::load(Exception& ex, const string& rootPath, DatabaseLoader& loader,bool disableTransaction) {
	flush();
	_disableTransaction = disableTransaction;
	FileSystem::MakeDirectory(_rootPath = rootPath);
	bool result = loadDirectory(ex, _rootPath , "", loader);
	_disableTransaction = false;
	return result;
}

bool Database::add(Exception& ex, const string& path, const UInt8* value, UInt32 size) {
	if (_disableTransaction)
		return true;
	lock_guard<mutex> lock(_mutex);
	if (!running() && !start(ex,Startable::PRIORITY_LOWEST))
		return false;
	_entries.emplace_back(_poolBuffers,path,value,size);
	wakeUp();
	return true;
}

bool Database::remove(Exception& ex, const string& path) {
	if (_disableTransaction)
		return true;
	lock_guard<mutex> lock(_mutex);
	if (!running() && !start(ex, Startable::PRIORITY_LOWEST))
		return false;
	_entries.emplace_back(_poolBuffers,path);
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
	EVP_Digest(entry.pBuffer->data(), entry.pBuffer->size(), result, NULL, EVP_md5(), NULL);
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
	ofile.write((const char*)entry.pBuffer->data(), entry.pBuffer->size());
}


bool Database::loadDirectory(Exception& ex, const string& directory, const string& path, DatabaseLoader& loader) {
	Exception ignore; // no exception here, it can be created more later (simply "no data" boolean returned)
	Files files(ignore, directory);
	if (ignore)
		return false; //no data here

	string name;
	bool hasData = false;
	for (const string& item : files) {
		string file(item);
		if (FileSystem::Exists(FileSystem::MakeDirectory(file))) {
			/// directory
			hasData = loadDirectory(ex, file, path + "/" + FileSystem::GetName(file,name), loader);
			continue;
		}

		/// file
		if (file.size() != 32) {
			// just erase the file
			FileSystem::Remove(file);
			continue;
		}

		// read the file
		ifstream ifile(FileSystem::MakeFile(file), ios::in | ios::binary | ios::ate);
		if (!ifile.good()) {
			ex.set(Exception::FILE, "Impossible to read file ", file);
			continue;
		}
		UInt32 size = (UInt32)ifile.tellg();
		ifile.seekg(0);
		vector<char> buffer(size);
		if (size>0)
			ifile.read(buffer.data(), size);
		// compute md5
		UInt8 result[16];
		EVP_Digest(buffer.data(), size, result, NULL, EVP_md5(), NULL);
		string value;
		Util::FormatHex(result, sizeof(result), value);
		// compare with file name
		if (FileSystem::GetName(file,name) != value) {
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
