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

#include "cppprofiler.h"

#include <stdint.h>
#include <memory>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <string.h>

using namespace CppProfiler;

class Profiler
{
public:
	typedef std::chrono::high_resolution_clock hclock;

	~Profiler();
	Profiler();
	void _StartModule (const char *module_name);
	void _EndModule();
	void _Flush();

	char *buf;
	int buf_size;
	int buf_pos;

	int _thread_id_;

	FILE *_fd_;
	thread_local static std::unique_ptr<Profiler> _profiler_;
	thread_local static hclock::time_point _internal_start_;
	static int id;
};

#define CLOCK_TIME(clk) std::chrono::duration_cast<std::chrono::nanoseconds>(clk.time_since_epoch()).count()

const int internal_size = sizeof (uint64_t) + sizeof (RECORD_TYPE) + sizeof (uint64_t);
Profiler::hclock::time_point _profiling_start_ = Profiler::hclock::now();

thread_local std::unique_ptr<Profiler> Profiler::_profiler_ (new Profiler);
thread_local Profiler::hclock::time_point Profiler::_internal_start_;

int Profiler::id = 0;


//Interface functions implementation
void CppProfiler::startModule (const char *module_name)
{
	Profiler::_internal_start_ = Profiler::hclock::now();
	Profiler::_profiler_->_StartModule (module_name);
}

void CppProfiler::endModule()
{
	Profiler::_internal_start_ = Profiler::hclock::now();
	Profiler::_profiler_->_EndModule();
}

void CppProfiler::flushProfiling()
{
	Profiler::_profiler_->_Flush();
}

Profiler::Profiler() :
	buf (nullptr),
	buf_size (CPP_PROFILING_CACHE_SIZE),
	buf_pos (0),
	_thread_id_ (id++)
{
	buf = new char[buf_size];
	memset (buf, 0, buf_size);

	FileHeader header;
	header.start_time = std::chrono::duration_cast<std::chrono::nanoseconds> (_profiling_start_.time_since_epoch()).count();

	memcpy (buf, (char *) &header, sizeof (header));
	buf_pos += sizeof (header);

	const char fname_fmt[] = CPP_PROFILING_FILENAME_PREFIX"_%ld_%d.cppprof";
	char fname[1024];

	std::chrono::nanoseconds start_time_point = std::chrono::duration_cast<std::chrono::nanoseconds> (_profiling_start_.time_since_epoch());
	start_time_point.count();

	snprintf (fname, 1024, fname_fmt, start_time_point.count(), _thread_id_);

	_fd_ = fopen (fname, "wb");
}

Profiler::~Profiler()
{
	_Flush();

	if (_fd_)
	{
		fclose (_fd_);
	}

	delete[] buf;
}

inline void write_internal (char *buf, uint64_t start, uint64_t end, RECORD_TYPE r_type = RECORD_TYPE::INTERNAL_RECORD)
{
	* ( (RECORD_TYPE *) buf) = RECORD_TYPE::INTERNAL_RECORD;
	buf += sizeof (RECORD_TYPE);

	* ( (uint64_t *) buf) = start;
	buf += sizeof (uint64_t);

	* ( (uint64_t *) buf) = end;
}

void Profiler::_StartModule (const char *module_name)
{
	int cur_pos (buf_pos);
	buf_pos += internal_size;

	int mod_name_length = strlen (module_name) + 1;
	int record_size = mod_name_length + sizeof (RECORD_TYPE) + sizeof (uint64_t) + sizeof (uint16_t);

	if (buf_pos + record_size > buf_size)
	{
		_Flush();
		cur_pos = buf_pos;
		buf_pos += internal_size;
	}

	char *buf_pointer = buf + buf_pos;
	buf_pos += record_size;

	* ( (RECORD_TYPE *) buf_pointer) = RECORD_TYPE::MODULE_START;
	buf_pointer += sizeof (RECORD_TYPE);

	* ( (uint16_t *) buf_pointer) = mod_name_length;
	buf_pointer += sizeof (uint16_t);

	memcpy (buf_pointer, module_name, mod_name_length);
	buf_pointer += mod_name_length;

	uint64_t now = CLOCK_TIME (hclock::now());
	* ( (uint64_t *) buf_pointer) = now;

	write_internal (buf + cur_pos, CLOCK_TIME (_internal_start_), now);
}

void Profiler::_EndModule()
{
	int cur_pos (buf_pos);
	buf_pos += internal_size;

	int record_size = sizeof (RECORD_TYPE) + sizeof (uint64_t);

	if (buf_pos + record_size > buf_size)
	{
		_Flush();
		cur_pos = buf_pos;
		buf_pos += internal_size;
	}

	char *buf_pointer = buf + buf_pos;
	buf_pos += record_size;

	* ( (RECORD_TYPE *) buf_pointer) = RECORD_TYPE::MODULE_END;
	buf_pointer += sizeof (RECORD_TYPE);

	uint64_t now = CLOCK_TIME (hclock::now());
	* ( (uint64_t *) buf_pointer) = now;

	write_internal (buf + cur_pos, CLOCK_TIME (_internal_start_), now);
}

void Profiler::_Flush()
{
	if (!_fd_) return;

	_internal_start_ = hclock::now();

	int written = fwrite (buf, 1, buf_pos, _fd_);
	written++;

	uint64_t internal_time = std::chrono::duration_cast<std::chrono::nanoseconds> (hclock::now() - _internal_start_).count();

	write_internal (buf, CLOCK_TIME (_internal_start_), internal_time, RECORD_TYPE::DATA_DUMP);

	buf_pos = internal_size;
}
