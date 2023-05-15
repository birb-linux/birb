#pragma once
#include <string>
#include <unordered_map>
#include <vector>

struct birb
{
	static std::vector<std::string> split_string(std::string text, std::string delimiter);
	static std::vector<std::string> read_file(std::string file_path);
	static std::string read_pkg_variable(std::string pkg_name, std::string var_name, std::string repo_path);
	static inline std::unordered_map<std::string, std::string> var_cache;
};
