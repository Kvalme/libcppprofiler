#define ENABLE_PROFILING 1
#include "cppprofiler.h"
#include <iostream>
#include <unistd.h>

int main()
{
	CPPProfiler::Profiler::init();
	PROFILE_FUNCTION
	
	for(int a = 0; a < 10000; ++a)
	{
		PROFILE_START("usleep(100)");
		usleep(100);
		PROFILE_END
	}
}
