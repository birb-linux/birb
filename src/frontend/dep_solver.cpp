/**************************************************/
/* Dependency solver for the birb package manager */
/**************************************************/

#include "Utils.hpp"
#include "Dependencies.hpp"
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>


enum Mode
{
	DEPENDENCY_SOLVER,
	REVERSE_DEPENDENCY_SOLVER,
	ORPHAN_FINDER,
};


int main(int argc, char** argv)
{
	/* Name of the package that we are inspecting */
	std::string pkg_name;

	/* Location where we can find all of the packages in */
	std::vector<pkg_source> repos;

	/* Dependency resolution direction */
	Mode operation_mode = DEPENDENCY_SOLVER;

	/* Testing args */
	if (argc == 4)
	{
		if (!strcmp(argv[1], "--test"))
		{
			repos.clear();

			repos.push_back(pkg_source("Testing repo", "0.0.0.0", argv[2]));
			pkg_name 		= argv[3];
		}
	}
	else if (argc == 3) /* Some extra argument + package name */
	{
		if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--reverse"))
		{
			operation_mode = REVERSE_DEPENDENCY_SOLVER;
		}
		else /* No valid argument was given */
		{
			std::cout << "Invalid argument: " << argv[1] << "\n";
			exit(1);
		}

		pkg_name = argv[2];
		repos = birb::get_pkg_sources();
	}
	else if (argc == 2)
	{
		if (!strcmp(argv[1], "-o") || !strcmp(argv[1], "--orphan"))
		{
			operation_mode = ORPHAN_FINDER;
		}
		else
		{
			/* Only the package name was provided */
			pkg_name = argv[1];
		}

		repos = birb::get_pkg_sources();
	}
	else
	{
		std::cout << "Incorrect amount of arguments!\n";
		return 1;
	}

	/* Read in the list of meta-packages and parse them into a map */
	std::vector<std::string> meta_package_list;

	/* Loop through all repos to get all meta_packages */
	assert(repos.size() > 0);
	std::vector<std::string> tmp_meta;
	std::string meta_path;
	for (pkg_source s : repos)
	{
		assert(s.path.empty() == false);
		assert(s.name.empty() == false);
		assert(s.url.empty()  == false);
		assert(s.is_valid()   == true);

		/* Check if the repo has a meta_package file */
		meta_path = s.path + "/meta_packages";
		if (!std::filesystem::exists(meta_path))
			continue;

		tmp_meta = birb::read_file(meta_path);
		meta_package_list.insert(meta_package_list.end(), tmp_meta.begin(), tmp_meta.end());
	}

	for (std::string line : meta_package_list)
	{
		/* Find the delimiter */
		size_t pos = line.find(":");

		/* Skip the line if the delimiter couldn't be found */
		if (pos == std::string::npos)
			continue;

		/* Get the key and the corresponding value and assign it into a map */
		birb::meta_packages[line.substr(0, pos)] = birb::split_string(line.substr(pos + 1, line.size() - (pos + 1)), " ");
	}

	std::vector<std::string> dependencies;

	switch (operation_mode)
	{
		/* Resolve dependencie normally for the given package */
		case (DEPENDENCY_SOLVER):
			/* Get all package dependencies recursively */
			dependencies = birb::get_dependencies(pkg_name, repos, 512);

			/* Deduplicate the dependency list */
			dependencies = birb::deduplicated_dep_list(dependencies);
			break;


		/* Find packages that depend on this given package */
		case (REVERSE_DEPENDENCY_SOLVER):
			dependencies = birb::get_reverse_dependencies(pkg_name, repos);
			break;

		case (ORPHAN_FINDER):
			birb::orphan_finder(repos);
			return 0;
			break;
	};

	for (std::string d : dependencies)
		std::cout << d << "\n";

	return 0;
}
