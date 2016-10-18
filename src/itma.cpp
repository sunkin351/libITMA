#include "itma.h"

#include "itma_implementation.hpp"
using namespace ITMA;

Context* CreateContext()
{
	return reinterpret_cast<Context*>(new MContext());
}

void DestroyContext(Context* context)
{
	delete reinterpret_cast<MContext*>(context);
}

Channel* CreateChannel(Context* context)
{
	MContext* ctx = reinterpret_cast<MContext*>(context);
	return reinterpret_cast<Channel*>(new MChannel(*ctx));
}

void DestroyChannel(Channel* channel)
{
	delete reinterpret_cast<MChannel*>(channel);
}

void OpenChannel(Channel* channel, unsigned int ChannelNumber)
{
	(reinterpret_cast<MChannel*>(channel))->open(ChannelNumber);
}

void CloseChannel(Channel* channel)
{
	(reinterpret_cast<MChannel*>(channel))->close();
}

void SendChannel(Channel* channel, void* data, char* signature, unsigned long long datasize)
{
	(reinterpret_cast<MChannel*>(channel))->send(data, signature, datasize);
}

unsigned long long RecieveChannel(Channel* channel, void** data)
{
	return (reinterpret_cast<MChannel*>(channel))->recieve(data);
}

bool PollChannel(Channel* channel)
{
	return (reinterpret_cast<MChannel*>(channel))->poll();
}
