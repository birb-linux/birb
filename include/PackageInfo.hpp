#pragma once

#include "Database.hpp"

#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace birb
{
	enum class pkg_flag
	{
		x86, x86_test, test, important, python, wip, font, proprietary
	};

	const static inline std::unordered_map<std::string, pkg_flag> pkg_flag_string_mappings = {
		{ "32bit", pkg_flag::x86 },
		{ "test32", pkg_flag::x86_test },
		{ "test", pkg_flag::test },
		{ "important", pkg_flag::important },
		{ "python", pkg_flag::python },
		{ "wip", pkg_flag::wip },
		{ "font", pkg_flag::font },
		{ "proprietary", pkg_flag::proprietary }
	};

	// check if the package is valid in all regards
	__attribute__((warn_unused_result))
	bool validate_package(const std::string& pkg_name);

	// check if the package name only contains allowed characters
	__attribute__((warn_unused_result))
	bool is_valid_package_name(const std::string& pkg_name);

	// figure out which repository the package is in
	__attribute__((warn_unused_result))
	std::optional<pkg_source> locate_package(const std::string& pkg_name);

	__attribute__((warn_unused_result))
	std::unordered_set<pkg_flag> get_pkg_flags(const std::string& pkg_name, const pkg_source& repo);
}
