#include <regex>

#include "Logging.hpp"
#include "PackageInfo.hpp"
#include "Utils.hpp"

const static std::regex package_name_regex("^[0-9a-z_+-]*$");

static std::optional<std::vector<pkg_source>> repo_list;

namespace birb
{
	bool validate_package(const std::string& pkg_name)
	{
		return is_valid_package_name(pkg_name) && locate_package(pkg_name)->is_valid() && !pkg_name.empty();
	}

	bool is_valid_package_name(const std::string& pkg_name)
	{
		return std::regex_match(pkg_name, package_name_regex);
	}

	std::optional<pkg_source> locate_package(const std::string& package_name)
	{
		// load the repo list if it hasn't been loaded get
		if (!repo_list.has_value())
			repo_list = birb::get_pkg_sources();

		const pkg_source repo = birb::locate_pkg_repo(package_name, repo_list.value());

		// package was found, return the repository
		if (repo.is_valid())
			return repo;

		// package could not be found, return an empty result
		return {};
	}

	std::unordered_set<pkg_flag> get_pkg_flags(const std::string& pkg_name, const pkg_source& repo)
	{
		const std::string flag_str = read_pkg_variable(pkg_name, pkg_variable::flags, repo.path);

		// if the flag_str is empty, there are no flags
		if (flag_str.empty())
			return {};

		const std::vector<std::string> split_flag_strings = split_string(flag_str, " ");

		// convert the strings to flag enum values

		std::unordered_set<pkg_flag> flags;
		for (const std::string& flag_str : split_flag_strings)
		{
			if (!pkg_flag_string_mappings.contains(flag_str))
			{
				warning("Package [", pkg_name, "] has an undefined flag: ", flag_str);
				continue;
			}

			flags.insert(pkg_flag_string_mappings.at(flag_str));
		}

		return flags;
	}
}
