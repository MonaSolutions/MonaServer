#pragma once
#include "Mona/PacketReader.h"
#include <fstream>
#include "Mona/AMF.h"
#include <vector>
#include <Mona/Task.h>
#include <Mona/WorkThread.h>

namespace Mona {
	class FlashWriter;
	class Invoker;
	class Publication;
	class PoolThread;
	class OutFileRTMPStream
	{
		double _timeBase = -1;
		uint32_t _timeOffset = 0;
		std::fstream _file;
		std::string _path;
		bool _appending;
	public:
		OutFileRTMPStream(const std::string& path, bool appending);
		void FeedData(UInt32 time, PacketReader* reader, AMF::ContentType type);
		~OutFileRTMPStream();
	};

	class InFileRTMPStream : public WorkThread, private Task, public virtual Object
	{
		enum class Status : UInt8{
			Stop,Play,Complete,Seek
		}_status;
		std::string _filePath;
		std::ifstream _file;
		clock_t _startTime;
		std::vector<UInt8> buffer;
		Publication * _publication;
		UInt32 _length;
		char _currentType;
		UInt32 _timeStamp;
		FlashWriter* _writer;
		UInt32 _seekTime;
	public:
		PoolThread * pThread;
		double duration;
		InFileRTMPStream(const std::string& path, Invoker* invoker, Publication * p,FlashWriter* writer);
		~InFileRTMPStream();
		bool run(Exception& ex) override;
		void handle(Exception& ex)  override;
		bool process();
		void seek(UInt32 time);
		void paused(bool value);
	};
}
