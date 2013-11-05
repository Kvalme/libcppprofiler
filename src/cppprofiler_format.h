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
	uint64_t start_time;
};
#pragma pack(pop)
}
