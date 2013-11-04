#define CPP_ENABLE_PROFILING 1
#include "cppprofiler.h"
#include <iostream>
#include <unistd.h>

int main()
{
	CPP_PROFILE_FUNCTION
	
	for(int a = 0; a < 10000; ++a)
	{
		CPP_PROFILE_START("usleep(100)");
		usleep(100);
		CPP_PROFILE_END
	}
}
