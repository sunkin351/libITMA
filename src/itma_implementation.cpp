/*
	This file is a part of the ITMA (Inter-Thread Messaging Achitecture) project under the MPLv2.0 or later license.
	See file "LICENSE" or https://www.mozilla.org/en-US/MPL/2.0/ for details.
*/

#include "itma_implementation.hpp"

namespace ITMA
{
	MContext::MContext()
	{
		context = std::thread(std::bind(&MContext::ThreadStart, this)); //Start Message Manager on context creation.
		context.detach();
	}

	std::shared_ptr<pipe> MContext::CreatePipe(int chan)
	{
		lock.lock();
		size_t ret = pipes.size();
		pipes.push_back(std::make_shared<pipe>(pipe(chan)));
		lock.unlock();
		return pipes[ret];
	}

	void MContext::DestroyPipe(std::shared_ptr<pipe> & pip)
	{
		lock.lock();
		for (uint32_t i = 0; i != pipes.size(); i++)
		{
			std::shared_ptr<pipe> & ref = pipes[i];
			if (ref == pip)
			{
				if (!(ref.use_count() > 2))
				{
					pipes.remove(i);
				}
				pip.reset();
				break;
			}
		}
		lock.unlock();
	}

	void MContext::ThreadStart()
	{
		while (true)
		{
			lock.lock();
			if (pipes.size() != 0) //Check to see if there are pipes to process.
			{
				for (int a = 0; a != pipes.size(); ++a) //Primary loop looking in all pipes for messages that need moving.
				{
					if (!pipes[a]->out.empty()) //Check to see if there are messages to move.
					{
						Message msg; //buffer message
						pipes[a]->ctxpop(msg);
						uint32_t channel = pipes[a]->channel;

						for (int b = 0; b != pipes.size(); ++b) // Secondary loop to check all pipes for certain criteria such as channel and subscription
						{
							if (a != b) //Skips sender pipe
							{
								uint32_t temp = pipes[b]->channel;
								if (temp == channel)
								{
									if (msg.signature == "") // If message signature is blank, message will be given to everyone on the same channel as the sender
									{
										pipes[b]->ctxpush(msg);
										continue;
									}

									if (pipes[b]->subscription.size() == 0) // if there are no subscriptions on a pipe and the message DOES have a signature, then said pipe will be skipped.
									{
										continue;
									}

									// Else if a pipe does have subscriptions,
									// they will be stepped through and compared to the messages signature.
									// If there is a match, the message will be added to the pipe, and the loop will
									// exit prematurely.
									for (int c = 0; c != pipes[b]->subscription.size(); ++c)
									{
										if (pipes[b]->subscription[c] == msg.signature)
										{
											pipes[b]->ctxpush(msg);
											break;
										}
									}
								}
							}
						}
					}
				}
			}
			lock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(1)); //To keep CPU usage down while still being responsive.
		}
	}


	MChannel::MChannel(MContext & ctx) : _ctx(ctx)
	{
	}

	MChannel::~MChannel()
	{
		close();
	}

	void MChannel::open(int channelNumber)
	{
		//A channel object can only have one pipe at a time.
		if (pip)
			close();

		pip = _ctx.CreatePipe(channelNumber);
	}

	void MChannel::close()
	{
		if (pip)
		{
			while (!pip->out.empty())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
			_ctx.DestroyPipe(pip);
		}
	}

	void MChannel::send(void* data, std::string signature, uint64_t datasize)
	{
		pip->send(data, signature, datasize);
	}

	uint64_t MChannel::recieve(void** data)
	{
		Message msg;
		if(pip->recieve(msg))
		{
			*data = msg.data;
			return msg.size;
		}
		else{return 0;}
	}

	bool MChannel::poll()
	{
		return !pip->in.empty();
	}

	void MChannel::subscribe(std::string sub)
	{
		pip->subscription.push_back(sub);
	}

	void MChannel::unsubscribe(std::string sub)
	{
		for (auto it = pip->subscription.begin(); it != pip->subscription.end(); ++it)
		{
			if (*it == sub)
			{
				pip->subscription.erase(it);
				break;
			}
		}
	}


	pipe::pipe(int _channel)
	{
		channel = _channel;
	}

	pipe::pipe(const pipe & tocopy)
	{
		channel = tocopy.channel;
		subscription = tocopy.subscription;
	}

	pipe::pipe(pipe && src)
	{
		*this = std::move(src);
	}

	pipe::~pipe()
	{
		if (!in.empty())
		{
			in.clear();
		}

		if (!out.empty())
		{
			out.clear();
		}
		subscription.clear();
	}

	void pipe::operator=(pipe && src)
	{
		in = std::move(src.in);
		out = std::move(src.out);
		subscription = std::move(src.subscription);
	}

	void pipe::ctxpush(Message & msg)
	{
		lock.lock();
		in.push(msg);
		lock.unlock();
	}

	void pipe::ctxpop(Message & msg)
	{
		lock.lock();
		out.pop(msg);
		lock.unlock();
	}
}

