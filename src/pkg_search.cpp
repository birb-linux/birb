#include "Birb.hpp"
#include <iostream>
#include <string>
#include <vector>

#define REPO_PATH "/var/db/pkg"
#define PACKAGE_LIST_PATH "/var/lib/birb/packages"
#define NEST_PATH "/var/lib/birb/nest"

/* This program is supposed to take in a list of package names
 * and then fetch their descriptions and the installation status */

/* The output will be in the following format: `package;version;description [installed]` */

int main(int argc, char** argv)
{
	/* Read in the list of all existing packages */
	std::vector<std::string> packages = birb::read_file(PACKAGE_LIST_PATH);

	/* Read in the list of installed packages */
	std::vector<std::string> installed_packages = birb::read_file(NEST_PATH);

	/* Loop through the packages given as arguments and format the results */
	for (int i = 1; i < argc; ++i)
	{
		std::cout << argv[i] << ";";
		std::cout << birb::read_pkg_variable(argv[i], "DESC", REPO_PATH) << ";";
		std::cout << birb::read_pkg_variable(argv[i], "VERSION", REPO_PATH) << ";";

		/* Check if the package is installed */
		for (std::string package : installed_packages)
			if (argv[i] == package)
				std::cout << "[installed]";

		std::cout << "\n";
	}

	return 0;
}
