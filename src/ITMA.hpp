/*
	This file is a part of the ITMA (Inter-Thread Messaging Achitecture) project under the MPLv2.0 or later license.
	See file "LICENSE" or https://www.mozilla.org/en-US/MPL/2.0/ for details.
*/

#pragma once

#ifndef ITMA_HPP
#define ITMA_HPP

#include <thread>
#include <string>
#include <queue>
#include <memory>
#include <mutex>

//_ITMA_EXPORT definition
#ifdef ITMA_EXPORT
#	if defined(_WIN32)
#		define _ITMA_EXPORT __declspec(dllexport)
#	elif defined(__GNUC__)
#		define _ITMA_EXPORT __attribute__((visibility("default")))
#	else
#		define _ITMA_EXPORT
#	endif
#elif defined(_WIN32)
#	define _ITMA_EXPORT __declspec(dllimport)
#else
#	define _ITMA_EXPORT
#endif //MTMA_EXPORT

#include "custom_vector.h"
#include "custom_queue.h"

namespace ITMA
{
	class pipe;

	class _ITMA_EXPORT MContext
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
	class _ITMA_EXPORT Channel
	{
		std::shared_ptr<pipe> pip;
		MContext & _ctx;
	public:
		Channel(MContext& ctx);
		Channel(MContext & ctx, int ChannelNumber);
		~Channel();

		void open(int channelNumber);
		void close();

		template<typename T>
		void send(T & object, std::string signature = "", bool more = false)
		{
			pip->send(object, signature, more);
		}

		template<typename T>
		void recieve(T & object) 
		{
			while (pip->in.empty())
				std::this_thread::sleep_for(std::chrono::milliseconds(2)); //blocking read
			pip->recieve(object);
		}

		bool poll();

		void subscribe(std::string sub) ;
		void unsubscribe(std::string sub) ;
	};

	struct Message
	{
		std::string signature;
		std::shared_ptr<void> data;
		char type;
		bool more;
		int size;

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
			if(data) 
				data.reset();
		}

		Message & operator=(Message&& source)
		{
			signature = source.signature;
			data = source.data;
			type = source.type;
			more = source.more;
			size = source.size;
			source.clear();
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
			type = source.type;
			more = source.more;
			size = source.size;
		}

		void clear()
		{
			signature = "";
			data.reset();
			type = 0;
			more = 0;
			size = 0;
		}
	};

	class _ITMA_EXPORT pipe
	{
		CustomQueue<Message> in; //incoming messages
		CustomQueue<Message> out; //outgoing messages
		std::vector<std::string> subscription; //list of subscriptions for a pipe
		int channel; //channel for a pipe
		std::mutex lock; //Mutex for synchronous access to the queues.
		enum //Message type specifiers
		{
			string,
			_bool,
			_8bitS,
			_16bitS,
			_32bitS,
			_64bitS,
			_8bitU,
			_16bitU,
			_32bitU,
			_64bitU,
			_voidptr,
			CustomObject
		};
	public:
		//constructors
		pipe(int _channel);
		pipe(const pipe& tocopy);
		pipe(pipe&& src);

		//destructor
		~pipe();

		void operator=(pipe&& src);

		//Methods that convert to void* data
		void send(std::string data, std::string signature, bool more = false);
		void send(bool data, std::string signature, bool more = false);

		void send(char data, std::string signature, bool more = false);
		void send(short data, std::string signature, bool more = false);
		void send(int data, std::string signature, bool more = false);
		void send(long data, std::string signature, bool more = false);

		void send(unsigned char data, std::string signature, bool more = false);
		void send(unsigned short data, std::string signature, bool more = false);
		void send(unsigned int data, std::string signature, bool more = false);
		void send(unsigned long data, std::string signature, bool more = false);


		template<class T, int size = sizeof(T)>
		void send(T & object, std::string signature, bool more = false)
		{
			std::shared_ptr<T> pointer( new T(object) );
			send(pointer, CustomObject, signature, more, size);
		}

		//Methods that convert from void* data
		bool recieve(std::string & dest);
		bool recieve(bool& dest);

		bool recieve(char& dest);
		bool recieve(short& dest);
		bool recieve(int& dest);
		bool recieve(long& dest);

		bool recieve(unsigned char& dest);
		bool recieve(unsigned short& dest);
		bool recieve(unsigned int& dest);
		bool recieve(unsigned long& dest);

		template<class T, int size = sizeof(T)>
		bool recieve(T & dest)
		{
			Message msg;
			bool rc = recieve(msg);
			if (rc) {
				if (msg.type != CustomObject || size != msg.size)
				{
					throw std::exception("Message data not of the same data type.");
				}
				dest = *static_cast<T*>(msg.data.get());
			}
			return rc;
		}

	private:
		//Methods that send/recieve messages
		inline void send(std::shared_ptr<void> data, char type, std::string signature, bool more = false, int size = 0);
		inline bool recieve(Message & msg);

		//Methods that are used in the Message Managing context
		void ctxpush(Message & msg);
		void ctxpop(Message & msg);

		friend class MContext;
		friend class Channel;
	};

	
}//namespace MTMA

#endif //ITMA_HPP
