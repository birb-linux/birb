#include "Database.hpp"
#include "Logging.hpp"
#include "PackageSearch.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace birb
{
	void pkg_search(const std::vector<std::string>& packages, const path_settings& paths)
	{
		if (!std::filesystem::exists(paths.package_list()))
			error("Could not find the package list. Run 'birb --sync' to sync the repositories and update package cache.");

		// read in the list of all existing packages
		const std::vector<std::string> pkg_list = birb::read_file(paths.package_list());
		if (pkg_list.empty())
			error("Empty package cache");

		// read in the list of installed packages
		const std::vector<std::string> installed_packages = birb::get_installed_packages(paths);

		// get the list of repositories
		const std::vector<pkg_source> pkg_sources = birb::get_pkg_sources(paths);
		if (pkg_sources.empty())
			error("No valid package repositories were found\n");

		for (const std::string& pkg_name : packages)
		{
			// make sure that the package exists
			const pkg_source repo = birb::locate_pkg_repo(pkg_name, pkg_sources);
			if (!repo.is_valid())
			{
				std::cout << "Package [" << pkg_name << "] doesn't exist" << '\n';
				continue;
			}

			const std::string version 		= birb::read_pkg_variable(pkg_name, pkg_variable::version, repo.path);
			const std::string description 	= birb::read_pkg_variable(pkg_name, pkg_variable::desc, repo.path);

			std::cout 	<< pkg_name << ";"
						<< version << ";"
						<< description << ";";

			// check if the package is installed
			if (std::find(installed_packages.begin(), installed_packages.end(), pkg_name) != installed_packages.end())
				std::cout << "[installed]";

			std::cout << "\n";
		}
	}
}
