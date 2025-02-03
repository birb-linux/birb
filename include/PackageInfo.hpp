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

	enum class [[nodiscard]] package_validation_error
	{
		noerr,
		invalid_name,
		missing_package,
		invalid_repo,
		empty_name
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
	package_validation_error validate_package(const std::string& pkg_name);

	// check if the package name only contains allowed characters
	__attribute__((warn_unused_result))
	bool is_valid_package_name(const std::string& pkg_name);

	// figure out which repository the package is in
	__attribute__((warn_unused_result))
	std::optional<pkg_source> locate_package(const std::string& pkg_name);

	__attribute__((warn_unused_result))
	std::unordered_set<pkg_flag> get_pkg_flags(const std::string& pkg_name, const pkg_source& repo);

	// find and cache all metapackages
	void find_meta_packages();

	__attribute__((warn_unused_result))
	bool is_meta_package(const std::string& pkg_name);

	__attribute__((warn_unused_result))
	const std::vector<std::string>& expand_meta_package(const std::string& meta_pkg_name);
}
