/*
 * itmacpp.hpp
 *
 *  Created on: Oct 17, 2016
 *      Author: joshua
 */
#pragma once
#ifndef INCLUDE_ITMACPP_HPP_
#define INCLUDE_ITMACPP_HPP_

#include "../src/itma.h"
#include <string>
#include <assert.h>
#include <exception>
#include <cstdint>

namespace ITMA {

	//
	class Exception : std::exception
	{
	protected:
		const char* msg;
	public:
		Exception(){msg = "An Error occurred.";}
		Exception(const char* _msg) : msg(_msg){}
		virtual const char* what(){return msg;}
	};

	//Forward declaration for Channel class.
	class MChannel;

	//Context class
	class MContext
	{
		Context* ctx;
		friend class MChannel;
	public:
		MContext()
		{
			ctx = CreateContext();
		}

		~MContext()
		{
			DestroyContext(ctx);
		}
	};

	class MChannel
	{
		Channel* channel;
	public:
		MChannel(MContext & ctx)
		{
			channel = CreateChannel(ctx.ctx);
		}

		MChannel(MContext & ctx, uint32_t ChannelNumber)
		{
			channel = CreateChannel(ctx.ctx);
			open(ChannelNumber);
		}

		void open(unsigned int ChannelNumber)
		{
			OpenChannel(channel, ChannelNumber);
		}

		void close()
		{
			CloseChannel(channel);
		}

		bool poll()
		{
			return PollChannel(channel);
		}

		template<class T, uint64_t size = sizeof(T)>
		void send(T& object, std::string signature, bool DontCopy = false)
		{
			if(DontCopy)
			{
				SendChannel(channel, (void*) new T(std::move(object)), signature.data(), size);
			}
			else{
				SendChannel(channel, (void*) new T(object), signature.data(), size);
			}
		}

		template<class T, uint64_t size = sizeof(T)>
		bool recieve(T& dest)
		{
			void* data = nullptr;
			uint64_t rc = RecieveChannel(channel, data);
			assert(data != 0);
			if(rc != 0)
			{
				if(rc != size)
				{
					throw Exception("Message Data Size Mismatch.");
				}
				T* buf = static_cast<T*>(data);
				dest = std::move(*buf);
				delete buf;
			}
			return rc != 0;
		}
	};
}
#endif /* INCLUDE_ITMACPP_HPP_ */
















