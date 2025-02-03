#include <cassert>
#include <filesystem>
#include <regex>

#include "Logging.hpp"
#include "PackageInfo.hpp"
#include "Utils.hpp"

const static std::regex package_name_regex("^[0-9a-z_+-]*$");

static std::optional<std::vector<pkg_source>> repo_list;
static std::optional<std::unordered_map<std::string, std::vector<std::string>>> meta_packages;

namespace birb
{
	package_validation_error validate_package(const std::string& pkg_name)
	{
		const std::optional<pkg_source> repo = locate_package(pkg_name);
		if (!repo.has_value())
		{
			error("Package [", pkg_name, "] does not exist");
			return package_validation_error::missing_package;
		}

		if (!is_valid_package_name(pkg_name))
		{
			error("Package [", pkg_name, "] has an invalid name");
			return package_validation_error::invalid_name;
		}

		if (!locate_package(pkg_name).value().is_valid())
		{
			error("Came across an invalid repository when validating package [", pkg_name, "]");
			return package_validation_error::invalid_repo;
		}

		if (pkg_name.empty())
		{
			error("Empty package name string erro");
			return package_validation_error::empty_name;
		}

		return package_validation_error::noerr;
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

	void find_meta_packages()
	{
		// load the repo list if it hasn't been loaded yet
		if (!repo_list.has_value())
			repo_list = birb::get_pkg_sources();

		assert(repo_list.has_value());

		std::vector<std::string> meta_package_list;

		for (const pkg_source& repo : repo_list.value())
		{
			assert(repo.path.empty() == false);
			assert(repo.name.empty() == false);
			assert(repo.url.empty()  == false);
			assert(repo.is_valid()   == true);

			/* Check if the repo has a meta_package file */
			const std::string meta_path = repo.path + "/meta_packages";
			if (!std::filesystem::exists(meta_path))
				continue;

			const std::vector<std::string> meta_file = birb::read_file(meta_path);
			meta_package_list.insert(meta_package_list.end(), meta_file.begin(), meta_file.end());
		}

		meta_packages = std::unordered_map<std::string, std::vector<std::string>>{};
		assert(meta_packages.has_value());
		for (const std::string& line : meta_package_list)
		{
			/* Find the delimiter */
			const size_t pos = line.find(":");

			/* Skip the line if the delimiter couldn't be found */
			if (pos == std::string::npos)
				continue;

			/* Get the key and the corresponding value and assign it into a map */
			meta_packages.value()[line.substr(0, pos)] = birb::split_string(line.substr(pos + 1, line.size() - (pos + 1)), " ");
		}
	}

	bool is_meta_package(const std::string& pkg_name)
	{
		// load metapackages if they haven't been loaded yet
		if (!meta_packages.has_value())
			find_meta_packages();

		assert(meta_packages.has_value());
		return meta_packages.value().contains(pkg_name);
	}

	const std::vector<std::string>& expand_meta_package(const std::string& meta_pkg_name)
	{
		// load metapackages if they haven't been loaded yet
		if (!meta_packages.has_value())
			find_meta_packages();

		assert(meta_packages.has_value());

		// the caller is supposed to check if the package is a meta package
		// before calling this function
		assert(meta_packages.value().contains(meta_pkg_name));

		return meta_packages.value().at(meta_pkg_name);
	}
}
