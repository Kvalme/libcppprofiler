#include "cppprofiler.h"

#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

using namespace CPPProfiler;

#define CLOCK_TIME(clk) std::chrono::duration_cast<std::chrono::nanoseconds>(clk.time_since_epoch()).count()

clock::time_point _profiling_start_ = clock::now();
thread_local std::unique_ptr<Profiler> _profiler_;
thread_local clock::time_point _internal_start_;
static int id = 0;


Profiler::Profiler() :
	buf(nullptr),
	buf_size(PROFILING_CACHE_SIZE),
	buf_pos(0),
	_thread_id_(id++)
{
	buf = new char[buf_size];
	memset(buf, 0, buf_size);

	FileHeader header;
	header.start_time = std::chrono::duration_cast<std::chrono::nanoseconds>(_profiling_start_.time_since_epoch()).count();

	memcpy(buf, (char *)&header, sizeof(header));
	buf_pos += sizeof(header);

	const char fname_fmt[] = PROFILING_FILENAME_PREFIX"_%ld_%d.cppprof";
	char fname[1024];

	std::chrono::nanoseconds start_time_point = std::chrono::duration_cast<std::chrono::nanoseconds>(_profiling_start_.time_since_epoch());
	start_time_point.count();

	snprintf(fname, 1024, fname_fmt, start_time_point.count(), _thread_id_);

	_fd_ = open(fname, O_WRONLY | O_CREAT | O_APPEND, 0777);
}

Profiler::~Profiler()
{
	_Flush();
	
	if (_fd_ >= 0)
	{
		close(_fd_);
	}
	
	delete[] buf;
}

void Profiler::init()
{
	_profiler_.reset(new Profiler);
}

void Profiler::startModule(const char *module_name)
{
	_internal_start_ = clock::now();
	_profiler_->_StartModule(module_name);
}

void Profiler::endModule()
{
	_internal_start_ = clock::now();
	_profiler_->_EndModule();
}

inline int reserve_internal()
{
	return sizeof(uint64_t) + sizeof(Profiler::RECORD_TYPE) + sizeof(uint64_t);
}

inline void write_internal(char *buf, uint64_t start, uint64_t duration_ns, Profiler::RECORD_TYPE r_type = Profiler::RECORD_TYPE::INTERNAL_RECORD)
{
	Profiler::RECORD_TYPE *r = (Profiler::RECORD_TYPE *)buf;
	*r = Profiler::RECORD_TYPE::INTERNAL_RECORD;
	buf += sizeof(Profiler::RECORD_TYPE);

	uint64_t *s = (uint64_t *)buf;
	*s = start;
	buf += sizeof(uint64_t);
	
	uint64_t *dur = (uint64_t *)buf;
	*dur = duration_ns;
}

void Profiler::_StartModule(const char *module_name)
{
	int cur_pos = buf_pos;
	buf_pos += reserve_internal();

	int mod_name_length = strlen(module_name) + 1;
	int record_size = mod_name_length + sizeof(RECORD_TYPE) + sizeof(uint64_t) + sizeof(uint16_t);

	if (buf_pos + record_size > buf_size)
	{
		_Flush();
		cur_pos = buf_pos;
		buf_pos += reserve_internal();
	}

	char *buf_pointer = buf + buf_pos;
	buf_pos += record_size;

	*((RECORD_TYPE *)buf_pointer) = RECORD_TYPE::MODULE_START;
	buf_pointer += sizeof(RECORD_TYPE);

	*((uint16_t*)buf_pointer) = mod_name_length;
	buf_pointer += sizeof(uint16_t);
	
	memcpy(buf_pointer, module_name, mod_name_length);
	buf_pointer += mod_name_length;

	*((uint64_t *)buf_pointer) = CLOCK_TIME(clock::now());

	uint64_t internal_time = std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - _internal_start_).count();

	write_internal(buf + cur_pos, CLOCK_TIME(_internal_start_), internal_time);
}

void Profiler::_EndModule()
{
	int cur_pos = buf_pos;
	buf_pos += reserve_internal();

	int record_size = sizeof(RECORD_TYPE) + sizeof(uint64_t);

	if (buf_pos + record_size > buf_size)
	{
		_Flush();
		cur_pos = buf_pos;
		buf_pos += reserve_internal();
	}

	char *buf_pointer = buf + buf_pos;
	buf_pos += record_size;

	*((RECORD_TYPE *)buf_pointer) = RECORD_TYPE::MODULE_END;
	buf_pointer += sizeof(RECORD_TYPE);

	*((uint64_t *)buf_pointer) = CLOCK_TIME(clock::now());

	uint64_t internal_time = std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - _internal_start_).count();

	write_internal(buf + cur_pos, CLOCK_TIME(_internal_start_), internal_time);
}


void Profiler::flushProfiling()
{
	_profiler_->_Flush();
}

void Profiler::_Flush()
{
	if (_fd_ < 0)return;
	
	_internal_start_ = clock::now();

	int written = write(_fd_, buf, buf_pos);
	written++;

	uint64_t internal_time = std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - _internal_start_).count();
	
	write_internal(buf, CLOCK_TIME(_internal_start_), internal_time, RECORD_TYPE::DATA_DUMP);

	buf_pos = reserve_internal();
}


