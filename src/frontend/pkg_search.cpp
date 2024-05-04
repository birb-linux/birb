#include "Database.hpp"
#include "Utils.hpp"
#include <iostream>
#include <string>
#include <vector>

constexpr char PACKAGE_LIST_PATH[] = "/var/lib/birb/packages";

/* This program is supposed to take in a list of package names
 * and then fetch their descriptions and the installation status */

/* The output will be in the following format: `package;version;description [installed]` */

int main(int argc, char** argv)
{
	/* Read in the list of all existing packages */
	const std::vector<std::string> packages = birb::read_file(PACKAGE_LIST_PATH);
	if (packages.empty())
	{
		std::cout << "ERROR: Empty package cache\n";
		exit(2);
	}

	/* Read in the list of installed packages */
	const std::vector<std::string> installed_packages = birb::get_installed_packages();

	/* Get the list of repositories */
	const std::vector<pkg_source> pkg_sources = birb::get_pkg_sources();
	if (pkg_sources.empty())
	{
		std::cout << "ERROR: No valid package repositories were found\n";
		exit(3);
	}

	/* Set this value to 1 if something goes wrong */
	short return_value = 0;

	/* Loop through the packages given as arguments and format the results */
	for (int i = 1; i < argc; ++i)
	{
		/* Make sure that the package exists */
		const pkg_source repo = birb::locate_pkg_repo(argv[i], pkg_sources);
		if (!repo.is_valid())
		{
			std::cout << "Package " << argv[i] << " doesn't exist" << std::endl;
			return_value = 1;
			continue;
		}

		const std::string version 		= birb::read_pkg_variable(argv[i], "VERSION", repo.path);
		const std::string description 	= birb::read_pkg_variable(argv[i], "DESC", repo.path);

		std::cout << argv[i] << ";";
		std::cout << version << ";";
		std::cout << description << ";";

		/* Check if the package is installed */
		for (const std::string& package : installed_packages)
			if (argv[i] == package)
				std::cout << "[installed]";

		std::cout << "\n";
	}

	return return_value;
}
