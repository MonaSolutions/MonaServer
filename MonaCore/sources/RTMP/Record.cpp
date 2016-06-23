#include "Record.h"
#include <Mona/Logs.h>
#include "Mona/FlashStream.h"
#include "Mona/Publication.h"
#define GETTIME clock()*1000/CLOCKS_PER_SEC
namespace Mona {
	OutFileRTMPStream::OutFileRTMPStream(const std::string& path, bool appending) :
		_path(path),_appending(appending)
	{
		if (!FileSystem::Exists(_path))_appending = false;
		if (_appending)
		{
			_file.open(_path, std::ios_base::binary | std::ios_base::in);
			if (_file.fail()) {
				//FATAL("Unable to open file %s with mode (%s)", STR(_path), strerror(errno));
				return;
			}
			_file.seekg(-4, std::ios::end);
			uint32_t lastTagSize = 0;
			char* p = reinterpret_cast<char*>(&lastTagSize);
			_file.get(*(p + 3)).get(*(p + 2)).get(*(p + 1)).get(*p);
			_file.seekg(-static_cast<int32_t>(lastTagSize), std::ios::end);
			p = reinterpret_cast<char*>(&_timeOffset);
			_file.get(*(p + 2)).get(*(p + 1)).get(*p).get(*(p + 3));
			_file.close();
			_file.open(_path, std::ios_base::binary | std::ios_base::app);
		}
		else
		{
			_file.open(_path, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
			if (_file.fail()) {
				//FATAL("Unable to open file %s with mode (%s)", STR(_path), strerror(errno));
				return;
			}
			_file.write("FLV\x1\x5\x0\x0\x0\x9\0\0\0\0", 13);
			FeedData(0, nullptr, AMF::AUDIO);
			FeedData(0, nullptr, AMF::VIDEO);
		}
		_timeBase = -1;
	}
	void OutFileRTMPStream::FeedData(UInt32 time, PacketReader* reader, AMF::ContentType type)
	{
		if (_timeBase < 0)_timeBase = time;
		//if (!buffer.isLast)return;
		_file.put(type);
		UInt32 totalLength = reader? reader->available():0;
		uint32_t t = totalLength;
		const char* p = reinterpret_cast<const char *>(&t);
		_file.put(p[2]).put(p[1]).put(p[0]);
		t = time - _timeBase + _timeOffset;
		_file.put(p[2]).put(p[1]).put(p[0]).put(p[3]).write("\0\0\0", 3);
		if(reader)
		_file.write(reinterpret_cast<const char*>(reader->current()), totalLength);
		t = totalLength + 11;
		_file.put(p[3]).put(p[2]).put(p[1]).put(p[0]);
	}

	OutFileRTMPStream::~OutFileRTMPStream()
	{
		INFO("stop recording");
		_file.flush();
		_file.close();
	}
	
	InFileRTMPStream::InFileRTMPStream(const std::string& path,Invoker* invoker,Publication * p,FlashWriter* writer):WorkThread("ReadFlv"),Task(*invoker), _filePath(path), _startTime(0), _publication(p), _length(0), _currentType(0), _timeStamp(0), pThread(nullptr), _writer(writer)
	{
		_file.open(_filePath, std::ios_base::binary | std::ios_base::in);
		_file.seekg(-4, std::ios::end);
		uint32_t lastTagSize = 0;
		char* tempp = reinterpret_cast<char*>(&lastTagSize);
		_file.get(*(tempp + 3)).get(*(tempp + 2)).get(*(tempp + 1)).get(*tempp);
		_file.seekg(-static_cast<int32_t>(lastTagSize), std::ios::end);
		_file.get(*(tempp + 2)).get(*(tempp + 1)).get(*tempp).get(*(tempp + 3));
		duration = lastTagSize;
		INFO("duration:", duration);
		_file.seekg(13);
		p->flvReader = this;
		p->start("");
	}
	bool InFileRTMPStream::run(Exception& ex) {
		if(_status == Status::Stop)_status = Status::Play;
		while(process())std::this_thread::yield();
		return true;
	}
	void InFileRTMPStream::handle(Exception& ex) {
		if (_status == Status::Complete)
		{
			auto& onPlayStatus = _writer->writeAMFData("onPlayStatus");
			onPlayStatus.amf0 = true;
			onPlayStatus.beginObject();
			onPlayStatus.writeStringProperty("code", "NetStream.Play.Complete");
			onPlayStatus.writeStringProperty("level", "status");
			onPlayStatus.endObject();
			_writer->writeAMFStatus("NetStream.Play.Stop", "Play Stop");
		}else
		{
			PacketReader packet(buffer.data(), _length);
			switch (_currentType)
			{
				case AMF::AUDIO:
					_publication->pushAudio(_timeStamp, packet);
					break;
				case AMF::VIDEO:
					_publication->pushVideo(_timeStamp, packet);
					break;
				case AMF::DATA:
				{
					AMFReader dr(packet);
					_timeStamp = 0;
					_publication->pushData(dr);
				}
				break;
			}
		}
		_publication->flush();
	}
	InFileRTMPStream::~InFileRTMPStream()
	{
		_file.close();
	}
	void InFileRTMPStream::seek(UInt32 time)
	{
		_status = Status::Seek;
		_seekTime = time;
	}
	void InFileRTMPStream::paused(bool value)
	{
		_status = value?Status::Stop:Status::Play;
	}
	bool InFileRTMPStream::process()  {
		if(!_publication->running())return false;
		switch(_status)
		{
			case Status::Seek:
			{
				_file.seekg(13);
				char* p = reinterpret_cast<char*>(&_timeStamp);
				while (_status == Status::Seek)
				{
					_file.get(_currentType);
					_timeStamp = 0;
					_file.get(*(p + 2)).get(*(p + 1)).get(*p);
					_length = _timeStamp;
					_file.get(*(p + 2)).get(*(p + 1)).get(*p).get(*(p + 3));
					if (_currentType == AMF::DATA)
					{
						_file.seekg(3, std::ios_base::cur);
						buffer.resize(_length);
						_file.read(reinterpret_cast<char*>(buffer.data()), _length);
						waitHandle();
						_file.seekg(4, std::ios_base::cur);
						continue;
					}
					INFO(_timeStamp);
					if (_timeStamp>_seekTime)
					{
						_status = Status::Play;
						INFO("seek done!");
						_startTime = GETTIME - _timeStamp;
						_file.seekg(-8, std::ios_base::cur);
					}
					else
					{
						_file.seekg(_length + 7, std::ios_base::cur);
					}
					if (_file.eof())break;
					if(_file.fail())
					{
						int errsv = errno;
						break;
					}
				}
			}
				break;
			case Status::Stop:
				_startTime = GETTIME - _timeStamp;
				return true;
		}
		if (_file.eof()|| _file.fail())
		{
			_status = Status::Complete;
			waitHandle();
			return false;
		}
		auto now = GETTIME - _startTime;
		uint32_t temp;
		char* p = reinterpret_cast<char*>(&temp);
		//do
		if(now > _timeStamp)
		{
			_file.get(_currentType);
			temp = 0;
			_file.get(*(p + 2)).get(*(p + 1)).get(*p);
			_length = temp;
			_file.get(*(p + 2)).get(*(p + 1)).get(*p).get(*(p + 3));
			_timeStamp = temp;
			_file.seekg(3, std::ios_base::cur);
			if (_timeStamp && !_startTime) {
				_startTime = GETTIME - _timeStamp;
			}
			if (_length) {
				buffer.resize(_length);
				_file.read(reinterpret_cast<char*>(buffer.data()), _length);
				waitHandle();
			}
			_file.seekg(4, std::ios_base::cur);
		} //while (now > _timeStamp);
		return true;
	}
}