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
#include <thread>
#include <chrono>
#include <mutex>
#include <map>

//Configuration

//Size of the internal cache in bytes. Shouldn't be less then size of the maximum string. Default value 100Mb
#ifndef PROFILING_CACHE_SIZE
#define PROFILING_CACHE_SIZE 100 * 1024 * 1024
#endif

//Prefix for profiler output files
#ifndef PROFILING_FILENAME_PREFIX
#define PROFILING_FILENAME_PREFIX "CPPProf"
#endif


namespace CPPProfiler
{
#ifdef ENABLE_PROFILING
#define PROFILE_START(name) CPPProfiler::Profiler::startModule(name);
#define PROFILE_END CPPProfiler::Profiler::endModule();
#define PROFILE_FUNCTION CPPProfiler::ProfileHelper prof_helper__(__FUNCTION__);
#define PROFILE_BLOCK(varname, blockname) CPPProfiler::ProfileHelper name(blockname);
#else
#define PROFILE_START(name)
#define PROFILE_END
#define PROFILE_FUNCTION
#define PROFILE_BLOCK(varname, blockname)
#endif
typedef std::chrono::high_resolution_clock clock;


class Profiler
{
public:
    enum class RECORD_TYPE
    {
        MODULE_START = 0,
        MODULE_END,
        INTERNAL_RECORD = 0xFF,
        DATA_DUMP
    };

#pragma pack(push, 1)
    struct FileHeader
    {
        const char MAGIC[7] = {'C', 'P', 'P', 'P', 'R', 'O', 'F'};
        int64_t start_time;
    };
#pragma pack(pop)

    static void init();

    static void startModule(const char *module_name)
    {
#ifdef ENABLE_PROFILING
        _internal_start_ = clock::now();
        _profiler_->_StartModule(module_name);
#endif
    }

    static void endModule()
    {
#ifdef ENABLE_PROFILING
        _internal_start_ = clock::now();
        _profiler_->_EndModule();
#endif
    }

    static void flushProfiling()
    {
#ifdef ENABLE_PROFILING
        _profiler_->_Flush();
#endif
    }

    ~Profiler();

private:
    Profiler();
    void _StartModule(const char *module_name);
    void _EndModule();
    void _Flush();

    char *buf;
    int buf_size;
    int buf_pos;

    int _thread_id_;

    int _fd_;
    thread_local static std::unique_ptr<Profiler> _profiler_;
    thread_local static clock::time_point _internal_start_;
};

class ProfileHelper
{
public:
    ProfileHelper(const char *module_name)
    {
#ifdef ENABLE_PROFILING
        Profiler::startModule(module_name);
#endif
    }
    ~ProfileHelper()
    {
#ifdef ENABLE_PROFILING
        Profiler::endModule();
#endif
    }
};

}
