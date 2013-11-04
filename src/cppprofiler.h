/*************************************************************************
 * libcpprofiler - c++11 based intrusive profiler
 *
 * Copyright (c) 2013 Denis Biryukov <denis.birukov@gmail.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would
 *    be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 *    be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 ************************************************************************/

#pragma once

//Configuration

//Size of the internal cache in bytes. Shouldn't be less then size of the maximum string. Default value 100Mb
#ifndef CPP_PROFILING_CACHE_SIZE
#define CPP_PROFILING_CACHE_SIZE 100 * 1024 * 1024
#endif

//Prefix for profiler output files
#ifndef CPP_PROFILING_FILENAME_PREFIX
#define CPP_PROFILING_FILENAME_PREFIX "CPPProf"
#endif

#ifdef CPP_ENABLE_PROFILING
#define CPP_PROFILE_START(name) CppProfiler::startModule(name);
#define CPP_PROFILE_END CppProfiler::endModule();
#define CPP_PROFILE_FUNCTION CppProfiler::ProfileHelper prof_helper__(__FUNCTION__);
#define CPP_PROFILE_BLOCK(name) CppProfiler::ProfileHelper prof_block_helper__(name);
#else
#define CPP_PROFILE_START(name)
#define CPP_PROFILE_END
#define CPP_PROFILE_FUNCTION
#define CPP_PROFILE_BLOCK(name)
#endif

#include <stdint.h>

namespace CppProfiler
{
/**
 * Record ids to parse profiling information
 */
enum class RECORD_TYPE
{
	MODULE_START = 0,
	MODULE_END = 1,
	INTERNAL_RECORD = 0xFF,
	DATA_DUMP = 0x100
};

#pragma pack(push, 1)
struct FileHeader
{
	const char MAGIC[7] = {'C', 'P', 'P', 'P', 'R', 'O', 'F'};
	int64_t start_time;
};
#pragma pack(pop)


/**
 * Adds record to profiling data with module_name
 * @param module_name name that will be written into profiling data
 */
extern void startModule (const char *module_name);

/**
 * End last module
 */
extern void endModule();

/**
 * Dump all gathered profiling data on disk
 */
extern void flushProfiling();

class ProfileHelper
{
public:
	ProfileHelper (const char *module_name)
	{
#ifdef CPP_ENABLE_PROFILING
		startModule (module_name);
#endif
	}
	~ProfileHelper()
	{
#ifdef CPP_ENABLE_PROFILING
		endModule();
#endif
	}
};

}
