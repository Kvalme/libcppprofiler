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

#define PROFILE_START(name) CPPProfiler::Profiler::startModule(name);
#define PROFILE_END CPPProfiler::Profiler::endModule();
#define PROFILE_FUNCTION CPPProfiler::ProfileHelper prof_helper__(__FUNCTION__);

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

		static void startModule(const char *module_name);
		static void endModule();

		static void flushProfiling();

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
};

class ProfileHelper
{
	public:
		ProfileHelper(const char *module_name)
		{
			Profiler::startModule(module_name);
		}
		~ProfileHelper()
		{
			Profiler::endModule();
		}
};

}
