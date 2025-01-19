#pragma once
#include "Config.hpp"
#include "Database.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace birb
{
	// figure out the full list of packages to install given the packages the
	// user wants to install
	std::vector<std::string> resolve_dependencies(const std::vector<std::string>& packages);

	std::vector<std::string> get_dependencies(const std::string& pkg, const std::vector<pkg_source>& repos, int depth);
	std::vector<std::string> get_reverse_dependencies(const std::string& pkg_name, const std::vector<pkg_source>& repos);
	std::vector<std::string> deduplicated_dep_list(const std::vector<std::string>& dependencies);
	std::vector<std::string> find_orphan_packages(const std::vector<pkg_source>& repos, const path_settings& paths);

	inline std::unordered_map<std::string, std::vector<std::string>> dependency_cache;
	inline std::unordered_map<std::string, std::vector<std::string>> reverse_dependency_cache;
}
