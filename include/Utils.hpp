#pragma once

#include <vector>
#include <string>

namespace birb
{
	void root_check();
	bool argcmp(char* arg, int argc, const std::string& option, int required_arg_count);
	std::vector<std::string> split_string(std::string text, const std::string& delimiter);
	std::vector<std::string> read_file(const std::string& file_path);
}
