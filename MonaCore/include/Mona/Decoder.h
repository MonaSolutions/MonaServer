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
#include "Mona/Invoker.h"
#include "Mona/Logs.h"

namespace Mona {

namespace Events {
	template<typename DecodedType>
	struct OnDecoded : Event<void(DecodedType& decoded, const SocketAddress& address)> {};
	struct OnDecodedEnd : Event<void()> {};
	struct OnDecoding : Event<UInt32(Exception& ex, UInt8* data, UInt32 size)> {};
};

template<typename DecodedType>
class Decoder : public virtual Object,
	public Events::OnDecoded<DecodedType>,
	public Events::OnDecodedEnd {

public:
	Decoder(Invoker& invoker, const char* name) : poolBuffers(invoker.poolBuffers), _name(name), _pThread(NULL), _poolThreads(invoker.poolThreads), _pDecoding(new Decoding(invoker, name)),
		onDecoding([this](Exception& ex, UInt8* data, UInt32 size) { return decoding(ex, data, size); }) {

		_pDecoding->OnDecodedEnd::subscribe(*this);
		_pDecoding->Events::template OnDecoded<DecodedType>::subscribe(*this);
		_pDecoding->Events::OnDecoding::subscribe(onDecoding);
	}

	virtual ~Decoder() {
		_pDecoding->Events::OnDecoding::unsubscribe(onDecoding);
		_pDecoding->Events::template OnDecoded<DecodedType>::unsubscribe(*this);
		_pDecoding->OnDecodedEnd::unsubscribe(*this);
	}

	UInt32 decode(PoolBuffer& pBuffer) {	
		return decode(SocketAddress::Wildcard(), pBuffer);
	}

	UInt32 decode(const SocketAddress& address,PoolBuffer& pBuffer) {	
		Exception ex;
		UInt32 consumed(pBuffer.size());
		std::lock_guard<std::mutex> lock(_pDecoding->_inMutex);
		_pThread = _poolThreads.enqueue(ex, _pDecoding, _pThread);
		if (ex) {
			ERROR(_name, ", ", ex.error());
			pBuffer.release();
		} else {
			_pDecoding->_input.emplace_back(poolBuffers);
			_pDecoding->_input.back().swap(pBuffer);
			_pDecoding->_inAddresses.emplace_back(address);
		}
		return consumed;
	}

protected:

	const PoolBuffers& poolBuffers;

	template <typename ...Args>
	void receive(Args&&... args) { _pDecoding->receive(args ...); }

private:

	// return the rest
	virtual UInt32 decoding(Exception& ex, UInt8* data,UInt32 size) = 0;

	class Decoding : public WorkThread, private Task, public virtual Object,
		public Events::OnDecoding,
		public Events::OnDecoded<DecodedType>,
		public Events::OnDecodedEnd {
	public:

		class OutType : public virtual Object {
		public:
			template <typename ...Args>
			OutType(const PoolBuffers& poolBuffers,Args&&... args) : pBuffer(poolBuffers),decoded(args ...) {}

			DecodedType	decoded;
			PoolBuffer	pBuffer;
		};

		Decoding(Invoker& invoker, const char* name) : _pBuffer(invoker.poolBuffers), WorkThread(name), Task(invoker) {}

		virtual ~Decoding() {
			for (OutType* pOut : _output)
				delete pOut;
		}

		template <typename ...Args>
		void receive(Args&&... args) {
			_new = true;
			_pLastOut = new OutType(_pBuffer.poolBuffers,args ...);
			std::lock_guard<std::mutex> lock(_outMutex);
			_output.emplace_back(_pLastOut);
			_outAddresses.emplace_back(_address);
		}

		bool run(Exception& ex) {

			// Just one!

			_new = false;

			PoolBuffer pBuffer(_pBuffer.poolBuffers);
			{
				std::lock_guard<std::mutex> lock(_inMutex);
				ASSERT_RETURN(!_input.empty(),true)
				pBuffer.swap(_input.front());
				_input.pop_front();
				_address = _inAddresses.front();
				_inAddresses.pop_front();
			}

			// add general buffer in front
			if (!_pBuffer.empty()) {
				if (!pBuffer.empty())
					_pBuffer->append(pBuffer.data(),pBuffer.size());
				pBuffer.swap(_pBuffer);
				_pBuffer.release();
			}

			UInt32 consumed(0);
			
			while(consumed<pBuffer.size()) { // while everything is not consumed

				if (consumed) {
					// not execute the first loop!
					if (_pLastOut) {
						_pBuffer->resize(pBuffer.size()-consumed, false);
						memcpy(_pBuffer->data(), pBuffer->data()+consumed, _pBuffer->size());
						pBuffer->resize(consumed);
						_pLastOut->pBuffer.swap(pBuffer);
						pBuffer.swap(_pBuffer);
					} else
						pBuffer->clip(consumed);
				}

				Exception exc;
				_pLastOut = NULL;
				consumed = OnDecoding::raise<0xFFFFFFFF>(exc, pBuffer->data(),pBuffer->size());

				if (exc)
					ERROR(name, ", ", exc.error())
		
				if (!consumed) {
					_pBuffer.swap(pBuffer); // memorize the rest in general buffer
					break;
				}

				if (_pLastOut && consumed >= pBuffer.size()) {
					_pLastOut->pBuffer.swap(pBuffer);
					break;
				}

			}
		
			if (_new)
				waitHandle();

			return true;
		}


		void handle(Exception& ex) {
			OutType* pOut(NULL);
			SocketAddress address;

			while (true) {
				{
					std::lock_guard<std::mutex> lock(_outMutex);
					if (_output.empty())
						break;
					pOut = _output.front();
					_output.pop_front();
					address = _outAddresses.front();
					_outAddresses.pop_front();
				}
				Events::OnDecoded<DecodedType>::raise(pOut->decoded,address);
				delete pOut;
			}
			if (pOut) // at less one has been done
				Events::OnDecodedEnd::raise();
		}

		bool									_new;
		OutType*								_pLastOut;
		SocketAddress							_address;
		PoolBuffer								_pBuffer;

		std::mutex								_inMutex;
		std::deque<PoolBuffer>					_input;
		std::deque<SocketAddress>				_inAddresses;
		
		std::mutex								_outMutex;
		std::deque<OutType*>					_output;
		std::deque<SocketAddress>				_outAddresses;
	};

	typename Decoding::OnDecoding::Type			onDecoding;
	typename Decoding::OnDecoded::Type			onDecoded;
	typename Decoding::OnDecodedEnd::Type		onDecodeEnd;

	const std::shared_ptr<Decoding>	_pDecoding;

	PoolThread*										_pThread;
	PoolThreads&									_poolThreads;
	const char*										_name;
};


} // namespace Mona
