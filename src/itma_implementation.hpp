/*
 * This file is a part of the libITMA (Inter-Thread Messaging Architecture) project under the MPLv2.0 or later license.
 * See the file "LICENSE" or https://www.mozilla.org/en-US/MPL/2.0/ for details.
 */

#pragma once

#ifndef SRC_ITMA_IMPLEMENTATION_HPP_
#define SRC_ITMA_IMPLEMENTATION_HPP_

#include <thread>
#include <string>
#include <queue>
#include <memory>
#include <mutex>
#include <cstdint>

#include "custom_vector.h"
#include "custom_queue.h"

namespace ITMA
{
	class pipe;

	class  MContext
	{
		std::thread context;
		CustomVector<std::shared_ptr<pipe>> pipes;
		std::mutex lock;
	public:
		MContext();
		std::shared_ptr<pipe> CreatePipe(int chan);
		void DestroyPipe(std::shared_ptr<pipe> & pip);

	private:
		void ThreadStart();
	};

	//ZMQPP socket style setup
	class  MChannel
	{
		std::shared_ptr<pipe> pip;
		MContext & _ctx;
	public:
		MChannel(MContext& ctx);
		MChannel(MContext & ctx, int ChannelNumber);
		~MChannel();

		void open(int channelNumber);
		void close();

		void send(void* data, std::string signature = "", uint64_t datasize);

		uint64_t recieve(void** data);

		bool poll();

		void subscribe(std::string sub) ;
		void unsubscribe(std::string sub) ;
	};

	struct Message
	{
		std::string signature;
		void* data = nullptr;
		uint64_t size = 0;

		Message() {}

		Message(Message && msg)
		{
			*this = std::move(msg);
		}

		Message(Message& msg)
		{
			copy(msg);
		}

		Message(const Message& msg)
		{
			copy(const_cast<Message&>(msg));
		}

		~Message()
		{
		}

		Message & operator=(Message&& source)
		{
			signature = std::move(source.signature);
			data = std::move(source.data);
			size = std::move(source.size);
			return *this;
		}
		Message & operator=(Message& source)
		{
			copy(source);
			return *this;
		}

		inline void copy(Message& source)
		{
			signature = source.signature;
			data = source.data;
			size = source.size;
		}

		void clear()
		{
			signature = "";
			data = 0;
			size = 0;
		}
	};

	class pipe
	{
		CustomQueue<Message> in; //incoming messages
		CustomQueue<Message> out; //outgoing messages
		std::vector<std::string> subscription; //list of subscriptions for a pipe
		int channel; //channel for a pipe
		std::mutex lock; //Mutex for synchronous access to the queues.

	public:
		//constructors
		pipe(int _channel);
		pipe(const pipe& tocopy);
		pipe(pipe&& src);

		//destructor
		~pipe();

		void operator=(pipe&& src);

		inline void send(void* data, std::string signature, uint64_t size)
		{
			Message temp;
			temp.data = data;
			temp.signature = signature;
			temp.size = size;

			lock.lock();
			out.push(std::move(temp));
			lock.unlock();
		}

		inline bool recieve(Message & msg)
		{
			if(!in.empty())
			{
				lock.lock();
				in.pop(msg);
				lock.unlock();
				return true;
			}
			else{
				return false;
			}
		}


	private:

		//Methods that are used in the Message Managing context
		void ctxpush(Message & msg);
		void ctxpop(Message & msg);

		friend class MContext;
		friend class Channel;
	};


}//namespace MTMA


#endif /* SRC_ITMA_IMPLEMENTATION_HPP_ */
