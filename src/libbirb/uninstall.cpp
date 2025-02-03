#include "CLI.hpp"
#include "Database.hpp"
#include "Dependencies.hpp"
#include "Logging.hpp"
#include "PackageInfo.hpp"
#include "Symlink.hpp"
#include "Uninstall.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>

namespace birb
{
	void uninstall(const std::vector<std::string>& packages, const path_settings& paths)
	{
		// validate package names
		for (const std::string& pkg_name : packages)
		{
			if (validate_package(pkg_name, paths) != package_validation_error::noerr)
				exit(1);
		}

		// figure out if the packages we are trying to delete are even installed
		std::vector<std::string> installed_packages = get_installed_packages(paths);
		for (const std::string& pkg_name : packages)
		{
			if (std::find(installed_packages.begin(), installed_packages.end(), pkg_name) == installed_packages.end())
				error("Package [", pkg_name, "] is not installed, so it cannot be uninstalled");
		}

		// check package flags
		for (const std::string& pkg_name : packages)
		{
			const std::optional<pkg_source> repo = locate_package(pkg_name, paths);
			const std::unordered_set<pkg_flag> flags = get_pkg_flags(pkg_name, repo.value());

			if (flags.contains(pkg_flag::important))
			{
				warning("Package [", pkg_name, "] is an IMPORTANT package! Uninstalling it might render your system unusable");
				if (!confirmation_menu("Do you want to uninstall it anyway?", false))
					exit(1);
			}
		}

		// check reverse dependencies
		log("Checking reverse dependencies");
		const std::vector<pkg_source> repos = get_pkg_sources(paths);
		for (const std::string& pkg_name : packages)
		{
			const std::vector<std::string> reverse_deps = get_reverse_dependencies(pkg_name, repos, paths);

			// if the reverse dependency list is not empty, the package
			// probably shouldn't be uninstalled
			if (!reverse_deps.empty())
			{
				info("Cannot uninstall package [", pkg_name, "] because the following packages depend on it:\n");

				for (const std::string& rev_dep : reverse_deps)
					std::cout << "  " << rev_dep << '\n';

				std::cout << '\n';

				if (!confirmation_menu("Uninstall anyway?", false))
					exit(1);
			}
		}

		// check if Xorg is running
		const bool xorg_running = is_process_running("Xorg");

		// read in the package database
		std::vector<std::string> db_file = birb::read_birb_db(paths);

		// read in the nest file
		std::vector<std::string> nest_file = birb::read_file(paths.nest());

		// start uninstalling the packages
		for (const std::string& pkg_name : packages)
		{
			log("Uninstalling [", pkg_name, "]");
			if (xorg_running)
				set_win_title(std::format("uninstalling {}", pkg_name));

			const std::optional<pkg_source> repo = locate_package(pkg_name, paths);
			const std::unordered_set<pkg_flag> flags = get_pkg_flags(pkg_name, repo.value());

			// python packages need to be uninstalled with pip
			if (flags.contains(pkg_flag::python))
				exec_shell_cmd(std::format("yes | pip3 uninstall {}", pkg_name));

			unlink_package(pkg_name, paths);

			// remove the fakeroot
			assert(!paths.fakeroot.empty()); // this would cause an unfortunate situation
			std::filesystem::remove_all(paths.fakeroot + "/" + pkg_name);

			// remove the package from the db
			db_file.erase(std::remove_if(db_file.begin(), db_file.end(),
					[&pkg_name](const std::string& entry)
					{
						return birb::split_string(entry, ";")[0] == pkg_name;
					}
				), db_file.end());

			// remove the package from the nest file (if it is there)
			nest_file.erase(std::remove_if(nest_file.begin(), nest_file.end(),
					[&pkg_name](const std::string& entry)
					{
						return entry == pkg_name;
					}
				), nest_file.end());

			log("[", pkg_name, "] uninstalled");
		}

		// write the nest file and the database to disk
		std::ofstream db_ofstream(paths.database());
		std::ostream_iterator<std::string> output_iterator(db_ofstream, "\n");
		std::copy(db_file.begin(), db_file.end(), output_iterator);

		std::ofstream nest_ofstream(paths.nest());
		std::ostream_iterator<std::string> nest_iterator(nest_ofstream, "\n");
		std::copy(nest_file.begin(), nest_file.end(), nest_iterator);
	}
}
