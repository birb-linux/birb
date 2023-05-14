#pragma once
#include <string>
#include <unordered_map>

struct birb
{
	static std::string read_pkg_variable(std::string pkg_name, std::string var_name, std::string repo_path);
	static inline std::unordered_map<std::string, std::string> var_cache;
};
