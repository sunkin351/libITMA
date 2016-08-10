#include "..\src\ITMA.hpp"

#include <thread>
#include <chrono>
#include <iostream>

#define timepoint std::chrono::steady_clock::time_point
#define time_now() std::chrono::steady_clock::now()
#define duration std::chrono::duration<double>
#define duration_cast(x) std::chrono::duration_cast<duration>(x)

template<class T>
inline void setArr(T * arr, long ArrSize, T typeobj)
{
	for (int i = 0; i != ArrSize; ++i)
	{
		arr[i] = typeobj;
	}
}

struct numArrObj
{
	numArrObj(unsigned int num = 255)
	{
		for (int i = 0; i != 128; ++i)
		{
			arr[i] = num;
		}
	}

	unsigned int arr[128];
};

void secondThread(ITMA::MContext * ctx)
{
	ITMA::Channel channel(*ctx, 0);

	for (int i = 0; i != 500; ++i)
	{
		channel.send(numArrObj());
	}
}

int main()
{
	ITMA::MContext context;
	ITMA::Channel channel(context, 0);

	std::thread testThread(std::bind(&secondThread, &context));
	testThread.detach();

	std::cout << "Starting Test.\n";

	timepoint start = time_now();

	{
		numArrObj temp;
		for (int i = 0; i != 500; ++i)
		{
			channel.recieve(temp);
			std::cout << "Message " << (i + 1) << " Recieved.\n";
		}
	}
	timepoint end = time_now();
	duration result = duration_cast(end - start);

	std::cout << result.count() << " Seconds\n";
	std::cin.ignore();
}