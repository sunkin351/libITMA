/*
*
*
*
*
*
*
*
*
*
*/

#pragma once

#ifndef MTMA_HPP
#define MTMA_HPP

#include <thread>
#include <string>
#include <deque>
#include <queue>
#include <memory>
#include <mutex>

//Windows dll EXPORT definition
//TODO: add Cross-platform dynamic library macros
#ifdef MTMA_EXPORT
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __declspec(dllimport)
#endif //MTMA_EXPORT

/* TODO: Make EXPORT macro compatible with Unix compilers for dynamic link library creation. */

namespace ITMA
{
	class pipe;

	class EXPORT MContext
	{
		std::thread context;
		std::deque<std::shared_ptr<pipe>> pipes;
		std::mutex mtx;
	public:
		MContext();
		std::shared_ptr<pipe> CreatePipe(int chan);
		void DestroyPipe(std::shared_ptr<pipe> & pip);

	private:
		void ThreadStart();
	};

	//ZMQPP socket style setup
	class EXPORT Channel
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

		void subscribe(std::string sub);
		void unsubscribe(std::string sub);
	};

	struct Message
	{
		std::string signature;
		std::shared_ptr<void> data;
		char type;
		bool more;
		int size;

		Message() {}

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
			copy(source);
			return *this;
		}
		Message & operator=(Message& source)
		{
			copy(source);
			return *this;
		}

		void copy(Message& source)
		{
			signature = source.signature;
			data = source.data;
			type = source.type;
			more = source.more;
			size = source.size;
		}
	};

	class EXPORT pipe
	{
		std::queue<Message> in; //incoming messages
		std::queue<Message> out; //outgoing messages
		std::vector<std::string> subscription; //list of subscriptions for a pipe
		int channel; //channel for a pipe
		std::mutex mtx; //Mutex for synchronous access to the queues.
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
		pipe(int _channel = 0);
		pipe(const pipe& tocopy);

		//destructor
		~pipe();

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

		//Methods that control the mutex
		void lock();
		void unlock();

		//Methods that are used in the Message Managing context
		void ctxpush(Message & msg);
		void ctxpop(Message & msg);

		friend class MContext;
		friend class Channel;
	};

	
}//namespace MTMA

#endif //MTMA_HPP