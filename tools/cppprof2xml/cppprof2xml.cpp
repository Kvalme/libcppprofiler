#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>
#include "cppprofiler.h"

using namespace CPPProfiler;

std::string add_spacer(int depth)
{
	std::string d(depth, '\t');
	return d;
}

std::vector<uint64_t> start_times;

int module_depth = 0;
void proceed_data_dump(int inFile, std::ofstream &out)
{
	std::cerr << __FUNCTION__ << std::endl;
	uint64_t start, duration;
	if (read(inFile, &start, sizeof(start)) != sizeof(start))return;
	if (read(inFile, &duration, sizeof(duration)) != sizeof(duration))return;

	out << add_spacer(module_depth) << "<Internal type=\"" << (int)Profiler::RECORD_TYPE::DATA_DUMP << "\" start=\"" << start << "\" duration=\"" << duration << "\" />" << std::endl;
}

void proceed_internal_record(int inFile, std::ofstream &out)
{
	std::cerr << __FUNCTION__ << std::endl;
	uint64_t start, duration;
	if (read(inFile, &start, sizeof(start)) != sizeof(start))return;
	if (read(inFile, &duration, sizeof(duration)) != sizeof(start))return;

	out << add_spacer(module_depth) << "<Internal type=\"" << (int)Profiler::RECORD_TYPE::DATA_DUMP << "\" start=\"" << start << "\" duration=\"" << duration << "\" />" << std::endl;
}

void proceed_module_start(int inFile, std::ofstream &out)
{
	std::cerr << __FUNCTION__ << std::endl;
	uint64_t start;
	uint16_t name_length;
	char *name;
	if (read(inFile, &name_length, sizeof(name_length)) != sizeof(name_length))return;
	name = new char[name_length];
	if (read(inFile, name, name_length) != name_length)return;
	if (read(inFile, &start, sizeof(start)) != sizeof(start))return;

	out << add_spacer(module_depth) << "<Module type=\"" << (int)Profiler::RECORD_TYPE::MODULE_START << "\" timestamp=\"" << start << "\" name=\"" << name << "\"/>" << std::endl;
	module_depth++;
	start_times.push_back(start);
}

void proceed_module_end(int inFile, std::ofstream &out)
{
	std::cerr << __FUNCTION__ << std::endl;
	uint64_t start;
	if (read(inFile, &start, sizeof(start)) != sizeof(start))return;

	uint64_t mod_start = start_times.back();
	start_times.pop_back();
	
	out << add_spacer(module_depth) << "<ModuleEnd timestamp=\"" << start << "\" duration=\""<<start- mod_start<<"\"/>" << std::endl;
	module_depth--;
	out << add_spacer(module_depth) << "</Module>" << std::endl;
}


void proceed_file(int inFile, std::ofstream &out)
{
	Profiler::FileHeader header, header_orig;

	if (read(inFile, &header, sizeof(header)) != sizeof(header))return;
	if (memcmp(header_orig.MAGIC, header.MAGIC, sizeof(header.MAGIC)) != 0)
	{
		throw std::runtime_error("Invalid magic bytes");
	}

	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	out << "<ProfileData timeStart=\"" << header.start_time << "\">" << std::endl;

	Profiler::RECORD_TYPE recordType;

	while (read(inFile, &recordType, sizeof(recordType)) == sizeof(recordType))
	{
		switch (recordType)
		{
			case Profiler::RECORD_TYPE::DATA_DUMP:
				proceed_data_dump(inFile, out);
				break;
			case Profiler::RECORD_TYPE::INTERNAL_RECORD:
				proceed_internal_record(inFile, out);
				break;
			case Profiler::RECORD_TYPE::MODULE_START:
				proceed_module_start(inFile, out);
				break;
			case Profiler::RECORD_TYPE::MODULE_END:
				proceed_module_end(inFile, out);
				break;
		}
	}

	while (module_depth > 0)
	{
		out << add_spacer(module_depth) << "</Module>" << std::endl;
		module_depth--;
	}

	out << "</ProfileData>" << std::endl;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage:\n" << argv[0] << " <cppprof_file>" << std::endl;
		return 1;
	}

	std::string in_file = argv[1];
	std::string out_file = in_file + ".xml";

	std::ofstream out(out_file);

	int inFile = open(in_file.c_str(), O_RDONLY);
	if (inFile < 0)
	{
		std::cerr << "Unable to open: " << argv[1] << std::endl;
		return 1;
	}

	try
	{
		proceed_file(inFile, out);
	}
	catch (std::runtime_error &err)
	{
		std::cerr << "Error:" << err.what() << std::endl;
	}


	return 0;
}
