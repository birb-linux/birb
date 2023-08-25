#pragma once
#include "Birb.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace birb
{
	std::vector<std::string> get_dependencies(const std::string& pkg, const std::vector<pkg_source>& repos, int depth);
	std::vector<std::string> get_reverse_dependencies(const std::string& pkg_name, const std::vector<pkg_source>& repos);
	std::vector<std::string> deduplicated_dep_list(const std::vector<std::string>& dependencies);
	void orphan_finder(const std::vector<pkg_source>& repos);

	inline std::unordered_map<std::string, std::vector<std::string>> meta_packages;
	inline std::unordered_map<std::string, std::vector<std::string>> dependency_cache;
	inline std::unordered_map<std::string, std::vector<std::string>> reverse_dependency_cache;
};
