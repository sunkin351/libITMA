#include "ITMA.hpp"


namespace ITMA
{
	inline void lock(std::mutex & mtx)
	{
		mtx.lock();
	}

	inline void unlock(std::mutex & mtx)
	{
		mtx.unlock();
	}


	MContext::MContext()
	{
		context = std::thread(std::bind(&MContext::ThreadStart, this)); //Start Message Manager on context creation.
		context.detach();
	}

	std::shared_ptr<pipe> MContext::CreatePipe(int chan)
	{
		lock(mtx);
		int ret = pipes.size();
		pipes.push_back(std::make_shared<pipe>(pipe(chan)));
		unlock(mtx);
		return pipes[ret];
	}

	void MContext::DestroyPipe(std::shared_ptr<pipe> & pip)
	{
		int i = 0;
		lock(mtx);
		for (auto it : pipes)
		{
			if (it == pip)
			{
				if (!(it.use_count() > 2))
				{
					pipes.erase(pipes.begin() + i);
				}
				pip.reset();
				break;
			}
			i++;
		}
		unlock(mtx);
	}

	void MContext::ThreadStart()
	{
		while (true)
		{
			lock(mtx);
			if (pipes.size() != 0) //Check to see if there are pipes to process.
			{
				for (int a = 0; a != pipes.size(); ++a) //Primary loop looking in all pipes for messages that need moving.
				{
					if (!pipes[a]->out.empty()) //Check to see if there are messages to move.
					{
						Message msg; //buffer message
						pipes[a]->ctxpop(msg);
						int channel = pipes[a]->channel;

						for (int b = 0; b != pipes.size(); ++b) // Secondary loop to check all pipes for certain criteria such as channel and subscription
						{
							if (a != b) //Skips sender pipe
							{
								if (pipes[b]->channel == channel)
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
			unlock(mtx);
			std::this_thread::sleep_for(std::chrono::milliseconds(1)); //To keep CPU usage down while still being responsive.
		}
	}


	Channel::Channel(MContext & ctx) : _ctx(ctx)
	{
	}

	Channel::Channel(MContext & ctx, int channel) : _ctx(ctx)
	{
		open(channel);
	}


	Channel::~Channel()
	{
		close();
	}

	void Channel::open(int channelNumber)
	{
		//A channel object can only have one pipe at a time.
		if (pip)
			_ctx.DestroyPipe(pip);

		pip = _ctx.CreatePipe(channelNumber);
	}

	void Channel::close()
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

	bool Channel::poll()
	{
		return !pip->in.empty();
	}

	void Channel::subscribe(std::string sub)
	{
		pip->subscription.push_back(sub);
	}

	void Channel::unsubscribe(std::string sub)
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

	pipe::~pipe()
	{
		while (!in.empty())
			in.pop();
		while (!out.empty())
			out.pop();

		subscription.clear();
	}

	void pipe::send(std::string dat, std::string sig, bool more)
	{
		std::shared_ptr<std::string> data(new std::string(dat));
		send(data, string, sig, more);
	}

	void pipe::send(bool data, std::string sig, bool more)
	{
		std::shared_ptr<bool> dat(new bool(data));
		send(dat, _bool, sig, more);
	}

	void pipe::send(char data, std::string sig, bool more)
	{
		std::shared_ptr<char> dat(new char(data));
		send(dat, _8bitS, sig, more);
	}

	void pipe::send(short data, std::string sig, bool more)
	{
		std::shared_ptr<short> dat(new short(data));
		send(dat, _16bitS, sig, more);
	}

	void pipe::send(int data, std::string sig, bool more)
	{
		std::shared_ptr<int> dat(new int(data));
		send(dat, _32bitS, sig, more);
	}

	void pipe::send(long data, std::string sig, bool more)
	{
		std::shared_ptr<long> dat(new long(data));
		send(dat, _64bitS, sig, more);
	}

	void pipe::send(unsigned char data, std::string sig, bool more)
	{
		std::shared_ptr<unsigned char> dat(new unsigned char(data));
		send(dat, _8bitU, sig, more);
	}

	void pipe::send(unsigned short data, std::string sig, bool more)
	{
		std::shared_ptr<unsigned short> dat(new unsigned short(data));
		send(dat, _16bitU, sig, more);
	}

	void pipe::send(unsigned int data, std::string sig, bool more)
	{
		std::shared_ptr<unsigned int> dat(new unsigned int(data));
		send(dat, _32bitU, sig, more);
	}

	void pipe::send(unsigned long data, std::string sig, bool more)
	{
		std::shared_ptr<unsigned long> dat(new unsigned long(data));
		send(dat, _64bitU, sig, more);
	}

	bool pipe::recieve(std::string & str)
	{
		Message msg;
		bool rc = recieve(msg);
		if (rc)
		{
			if (msg.type != string)
			{
				throw std::exception("Message data not of type string.");
			}
			str = *static_cast<std::string*>(msg.data.get());
		}
		return rc;
	}

	bool pipe::recieve(bool & data)
	{
		Message msg;
		bool rc = recieve(msg);
		if (rc) {
			if (msg.type != _bool)
			{
				throw std::exception("Message data not of type bool.");
			}
			data = *static_cast<bool*>(msg.data.get());
		}
		return rc;
	}

	bool pipe::recieve(char & dest)
	{
		Message msg;
		bool rc = recieve(msg);
		if (rc) {
			if (msg.type != _8bitS)
			{
				throw std::exception("Message data not of type char.");
			}
			dest = *static_cast<char*>(msg.data.get());
		}
		return rc;
	}

	bool pipe::recieve(short & dest)
	{
		Message msg;
		bool rc = recieve(msg);
		if (rc) {
			if (msg.type != _16bitS)
			{
				throw std::exception("Message data not of type short.");
			}
			dest = *static_cast<short*>(msg.data.get());
		}
		return rc;
	}

	bool pipe::recieve(int & dest)
	{
		Message msg;
		bool rc = recieve(msg);
		if (rc) {
			if (msg.type != _32bitS)
			{
				throw std::exception("Message data not of type int.");
			}
			dest = *static_cast<int*>(msg.data.get());
		}
		return rc;
	}

	bool pipe::recieve(long & dest)
	{
		Message msg;
		bool rc = recieve(msg);
		if (rc) {
			if (msg.type != _64bitS)
			{
				throw std::exception("Message data not of type long.");
			}
			dest = *static_cast<long*>(msg.data.get());
		}
		return rc;
	}

	bool pipe::recieve(unsigned char & dest)
	{
		Message msg;
		bool rc = recieve(msg);
		if (rc) {
			if (msg.type != _8bitU)
			{
				throw std::exception("Message data not of type unsigned char.");
			}
			dest = *static_cast<unsigned char*>(msg.data.get());
		}
		return rc;
	}

	bool pipe::recieve(unsigned short & dest)
	{
		Message msg;
		bool rc = recieve(msg);
		if (rc) {
			if (msg.type != _16bitU)
			{
				throw std::exception("Message data not of type unsigned short.");
			}
			dest = *static_cast<unsigned short*>(msg.data.get());
		}
		return rc;
	}

	bool pipe::recieve(unsigned int & dest)
	{
		Message msg;
		bool rc = recieve(msg);
		if (rc) {
			if (msg.type != _32bitU)
			{
				throw std::exception("Message data not of type unsigned int.");
			}
			dest = *static_cast<unsigned int*>(msg.data.get());
		}
		return rc;
	}

	bool pipe::recieve(unsigned long & dest)
	{
		Message msg;
		bool rc = recieve(msg);
		if (rc) {
			if (msg.type != _64bitU)
			{
				throw std::exception("Message data not of type unsigned long.");
			}
			dest = *static_cast<unsigned long*>(msg.data.get());
		}
		return rc;
	}

	void pipe::send(std::shared_ptr<void> data, char type, std::string sig, bool more, int size)
	{
		Message buffer;
		buffer.data = data;
		buffer.type = type;
		buffer.signature = sig;
		buffer.more = more;
		buffer.size = size;

		lock();
		out.push(buffer);
		unlock();
	}

	bool pipe::recieve(Message & msg)
	{
		if (!in.empty())
		{
			lock();
			msg = in.front();
			in.pop();
			unlock();
			return true;
		}
		else
		{
			return false;
		}
	}

	void pipe::lock()
	{
		ITMA::lock(mtx);
	}

	void pipe::unlock()
	{
		ITMA::unlock(mtx);
	}

	void pipe::ctxpush(Message & msg)
	{
		lock();
		in.push(msg);
		unlock();
	}

	void pipe::ctxpop(Message & msg)
	{
		lock();
		msg = out.front();
		out.pop();
		unlock();
	}
}
