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
#include "Mona/TCPClient.h"
#include "Mona/TCPServer.h"
#include "Mona/UDPSocket.h"
#include "Mona/Logs.h"
#include <list>

using namespace std;
using namespace Mona;

class Mutex {
public:
	Mutex(bool factice = false) : _factice(factice) {}
	void lock() { if (_factice) return;  _mutex.lock(); }
	void unlock() { if (_factice) return;  _mutex.unlock(); }

private:
	bool		_factice;
	std::mutex	_mutex;
};

class TCPEchoClient : public TCPClient {
public:
	TCPEchoClient(const SocketManager& manager,bool parallel) : testDisconnection(false),TCPClient(manager),_mutex(!parallel),parallel(parallel) {
		
		onData = [this](PoolBuffer& pBuffer)->UInt32 {
			lock_guard<Mutex> lock(_mutex);
			if (pBuffer->size() < _datas.front().size())
				return 0; // wait more data
			UInt32 consumed(_datas.front().size());
			CHECK(memcmp(_datas.front().data(), pBuffer->data(), consumed) == 0);
			_datas.pop_front();
			if (this->parallel)
				_signal.set();
			return consumed;
		};

		onError = [this](const Exception& ex) {
			if (!testDisconnection)
				FATAL_ERROR("TCPEchoClient, ", ex.error())
		};

		onDisconnection = [this](TCPClient& client,const SocketAddress& peerAddress){
			CHECK(!connected())
			_signal.set();
		};

		OnError::subscribe(onError);
		OnData::subscribe(onData);
		OnDisconnection::subscribe(onDisconnection);
	}

	~TCPEchoClient() {
		OnError::unsubscribe(onError);
		OnData::unsubscribe(onData);
		OnDisconnection::unsubscribe(onDisconnection);
	}
	

	const bool parallel;
	bool testDisconnection;


	bool join() {
		if (!parallel) {
			if (testDisconnection)
				return !connected();
			lock_guard<Mutex> lock(_mutex);
			return _datas.empty();
		}
		while(_signal.wait(20000)) {
			lock_guard<Mutex> lock(_mutex);
			if (testDisconnection)
				return !connected();
			if (_datas.empty())
				return true;
		}
		return false;
	}

	bool echo(Exception& ex,const UInt8* data,UInt32 size) {
		{
			lock_guard<Mutex> lock(_mutex);
			_datas.emplace_back(size);
			memcpy(_datas.back().data(), data, size);
		}
		return send(ex, data, size);
	}

private:
	
	deque<vector<UInt8>> _datas;
	Signal				_signal;
	Mutex				_mutex;

	OnData::Type			onData;
	OnError::Type			onError;
	OnDisconnection::Type	onDisconnection;
};




class UDPEchoClient {
public:
	UDPEchoClient(const SocketManager& manager,bool parallel) : _socket(manager) ,_mutex(!parallel),parallel(parallel) {
		onError = [this](const Exception& ex) {
			FATAL_ERROR("UDPEchoClient, ",ex.error());
		};
		onPacket = [this](PoolBuffer& pBuffer,const SocketAddress& address) {
			lock_guard<Mutex> lock(_mutex);
			CHECK(_address == address);
			CHECK(_datas.front().size() == pBuffer->size() && memcmp(_datas.front().data(), pBuffer->data(), pBuffer->size()) == 0);
			_datas.pop_front();
			if (this->parallel)
				_signal.set();
		};
		_socket.OnError::subscribe(onError);
		_socket.OnPacket::subscribe(onPacket);
	}
	
	const bool parallel;

	~UDPEchoClient() {
		_socket.OnPacket::unsubscribe(onPacket);
		_socket.OnError::unsubscribe(onError);
	}

	bool join() {
		if (!parallel) {
			lock_guard<Mutex> lock(_mutex);
			return _datas.empty();
		}
		while(_signal.wait(20000)) {
			lock_guard<Mutex> lock(_mutex);
			if (_datas.empty())
				return true;
		}
		return _datas.empty();
	}

	bool connect(Exception& ex, const SocketAddress& address) {
		bool result = _socket.connect(ex, address);
		lock_guard<Mutex> lock(_mutex);
		_address.set(address);
		return result;
	}

	bool echo(Exception& ex,const UInt8* data,UInt32 size) {
		{
			lock_guard<Mutex> lock(_mutex);
			_datas.emplace_back(size);
			memcpy(_datas.back().data(), data, size);
		}
		return _socket.send(ex, data, size);
	}

private:

	UDPSocket::OnError::Type	onError;
	UDPSocket::OnPacket::Type	onPacket;
	
	Mutex						_mutex;
	std::deque<vector<UInt8>>   _datas;
	Signal					    _signal;
	SocketAddress				_address;
	UDPSocket					_socket;
};


static Signal TaskEvent;
class TaskHandlerSockets : public TaskHandler {
public:
	TaskHandlerSockets() {
		start();
	}

	void stop() { TaskHandler::stop(); }

	template<typename SocketType>
	bool join(SocketType& socket) {
		if (socket.parallel)
			return socket.join();
		bool joined(false);
		while (!(joined=socket.join()) && TaskEvent.wait(20000)) {
			Exception ex;
			giveHandle(ex);
			CHECK(!ex);
		}
		return joined;
	}

private:
	void requestHandle() {
		TaskEvent.set();
	}
};

static string					Short0Data(1024, '\0');
static string					Long0Data(9999999, '\0');
static PoolThreads				Threads;
static PoolBuffers				Buffers;
static SocketManager			ParallelSockets(Buffers,Threads);
static TaskHandlerSockets		TaskSockets;
static SocketManager			Sockets(TaskSockets,Buffers,Threads);



class TCPConnection {
public:
	TCPConnection(const SocketAddress& peerAddress,SocketFile& file,const SocketManager& manager) : _client(peerAddress,file, manager) {
		onData = [this](PoolBuffer& pBuffer) {
			Exception ex;
			CHECK(_client.send(ex, pBuffer->data(), pBuffer->size()) && !ex);
			return pBuffer->size();
		};
		onError = [this](const Exception& ex) {
			FATAL_ERROR("Client, ",ex.error());
		};
		onDisconnection = [this](TCPClient& client, const SocketAddress& peerAddress){
			CHECK(!_client.connected());
		};
		_client.OnError::subscribe(onError);
		_client.OnData::subscribe(onData);
		_client.OnDisconnection::subscribe(onDisconnection);
	}

	~TCPConnection() {
		_client.OnError::unsubscribe(onError);
		_client.OnData::unsubscribe(onData);
		_client.OnDisconnection::unsubscribe(onDisconnection);
	}
private:
	TCPClient							_client;
	TCPClient::OnData::Type				onData;
	TCPClient::OnError::Type			onError;
	TCPClient::OnDisconnection::Type	onDisconnection;
};

void TCPTest(SocketManager& sockets) {

	TCPServer server(sockets);
	list<TCPConnection> connections;

	TCPServer::OnError::Type onError = [](const Exception& ex) {
		FATAL_ERROR("TCPServer, ",ex.error());
	};
	TCPServer::OnConnection::Type onConnection = [&](Exception& ex, const SocketAddress& address, SocketFile& file) {
		CHECK(address && file);
		connections.emplace_back(address, file, sockets);
	};
	server.OnError::subscribe(onError);
	server.OnConnection::subscribe(onConnection);

	Exception ex;
	SocketAddress host(IPAddress::Wildcard(),62435);

	CHECK(server.start(ex, host) && !ex);
	server.stop();
	CHECK(server.start(ex, host) && !ex);

	TCPEchoClient client(sockets, &sockets == &ParallelSockets);
	SocketAddress target(IPAddress::Loopback(),host.port());
	CHECK(client.connect(ex, target) && !ex && client.connected());
	CHECK(client.echo(ex,BIN EXPAND("hi mathieu and thomas")) && !ex);
	CHECK(client.echo(ex,BIN Long0Data.c_str(),Long0Data.size()) && !ex);
	CHECK(TaskSockets.join(client));

	client.testDisconnection = true;

	client.disconnect();

	CHECK(TaskSockets.join(client));

	// Test an unknown connection
	SocketAddress unknown(IPAddress::Loopback(),62434);
	if (client.connect(ex, unknown)) {
		CHECK(!ex)
		if (!client.parallel) {
			CHECK(client.connected());
			CHECK(client.address());
			CHECK(client.peerAddress()==unknown);
			CHECK(client.send(ex, BIN EXPAND("salut")) && !ex)
			CHECK(client.connected());
		}
		CHECK(TaskSockets.join(client));
	}
	CHECK(!client.connected());

	server.OnError::unsubscribe(onError);
	server.OnConnection::unsubscribe(onConnection);
}

void UDPTest(SocketManager& sockets) {
	UDPSocket    server(sockets);
	UDPSocket::OnError::Type onError([&](const Exception& ex) {
		FATAL_ERROR("UDPEchoServer, ",ex.error());
	});
	UDPSocket::OnPacket::Type onPacket([&](PoolBuffer& pBuffer,const SocketAddress& address) {
		Exception ex;
		CHECK(server.send(ex,pBuffer->data(), pBuffer->size(), address) && !ex);
	});
	server.OnError::subscribe(onError);
	server.OnPacket::subscribe(onPacket);

	Exception ex;
	SocketAddress	 host(IPAddress::Wildcard(),62435);

	CHECK(server.bind(ex, host) && !ex);
	server.close();
	CHECK(server.bind(ex, host) && !ex);

	UDPEchoClient    client(sockets,&sockets==&ParallelSockets);
	SocketAddress	 target(IPAddress::Loopback(),host.port());
	CHECK(client.connect(ex, target) && !ex);

	CHECK(client.echo(ex,BIN EXPAND("hi mathieu and thomas")) && !ex);
	CHECK(client.echo(ex,BIN Short0Data.c_str(),Short0Data.size()) && !ex);
	CHECK(TaskSockets.join(client));

	server.OnError::unsubscribe(onError);
	server.OnPacket::unsubscribe(onPacket);
}

ADD_TEST(SocketTest, StartSockets) {
	Exception ex;
	CHECK(Sockets.start(ex) && !ex && Sockets.running());
	CHECK(ParallelSockets.start(ex) && !ex && ParallelSockets.running());
}

ADD_TEST(SocketTest, TCPSocket) {
	TCPTest(Sockets);
}

ADD_TEST(SocketTest, TCPParallelSocket) {
	TCPTest(ParallelSockets);
}

ADD_TEST(SocketTest, UDPSocket) {
	UDPTest(Sockets);
}

ADD_TEST(SocketTest, UDPParallelSocket) {
	UDPTest(ParallelSockets);
}

ADD_TEST(SocketTest, StopSockets) {
	TaskSockets.stop();
	Sockets.stop();
	CHECK(!Sockets.running());
	ParallelSockets.stop();
	CHECK(!ParallelSockets.running());
}