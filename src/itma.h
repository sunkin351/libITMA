/*
	This file is a part of the ITMA (Inter-Thread Messaging Achitecture) project under the MPLv2.0 or later license.
	See file "LICENSE" or https://www.mozilla.org/en-US/MPL/2.0/ for details.
*/

//Windows DLL EXPORT Definition
#ifdef ITMA_WINDOWS

#	ifdef ITMA_EXPORT
#		define EXPORT __declspec(dllexport)
#	else
#		define EXPORT __declspec(dllimport)
#	endif

#else
#	define EXPORT
#endif

extern "C"{

//Context API
struct Context;
typedef struct Context Context;

EXPORT Context* CreateContext(); //Returns a new messaging context.
EXPORT void DestroyContext(Context* context);

//Channel API
struct Channel;
typedef struct Channel Channel;

EXPORT Channel* CreateChannel();
EXPORT void DestroyChannel(Channel * channel);

EXPORT void OpenChannel(Channel* channel, int ChannelNumber);
EXPORT void CloseChannel(Channel* channel);

EXPORT void SendChannel(Channel* channel, void* data, char* signature, unsigned long long datasize);
EXPORT unsigned long long RecieveChannel(Channel* channel, void** data);

EXPORT bool PollChannel(Channel* channel);

EXPORT void SubscribeChannel(Channel* channel, char* subscription);
EXPORT void UnsubscribeChannel(Channel* channel, char* subscription);
}
