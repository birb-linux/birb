#pragma once

#include <vector>
#include <string>

namespace birb
{
	std::vector<std::string> split_string(std::string text, const std::string& delimiter);
	std::vector<std::string> read_file(const std::string& file_path);
}
