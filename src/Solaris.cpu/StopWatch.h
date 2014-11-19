#ifndef STOPWATCH_H_
#define STOPWATCH_H_

#include <cstdint>

class StopWatch
{
public:
	//StopWatch();
	//~StopWatch();

	int64_t start();
	int64_t stop();
	int64_t getElapsedTime();

private:
	int64_t GetTimeMicro64();

	int64_t _start;
	int64_t _stop;
};

#endif
