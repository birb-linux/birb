/**************************************************/
/* Dependency solver for the birb package manager */
/**************************************************/

#include "Birb.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

static std::unordered_map<std::string, std::vector<std::string>> meta_packages;
static std::unordered_map<std::string, std::vector<std::string>> dependency_cache;

std::vector<std::string> get_dependencies(const std::string& pkg, const std::string& repo_path)
{
	std::vector<std::string> deps;

	/* Don't do anything if this is a meta-package */
	if (!meta_packages[pkg].empty())
		return deps;

	/* Check if this package has its dependencies in the cache already */
	if (!dependency_cache[pkg].empty())
		return dependency_cache[pkg];

	/* Read data from the package file */
	std::string dep_line = birb::read_pkg_variable(pkg, "DEPS", repo_path);

	/* Split the string */
	size_t pos = 0;
	while ((pos = dep_line.find(" ")) != std::string::npos)
	{
		std::string dep = dep_line.substr(0, pos);

		/* Check if the dependency is a meta package and should be expanded */
		if (!meta_packages[dep].empty())
		{
			/* Expand the meta package */
			deps.insert(deps.end(), meta_packages[dep].begin(), meta_packages[dep].end());
		}
		else
		{
			deps.push_back(dep);
		}

		dep_line.erase(0, pos + 1);
	}

	if (!dep_line.empty())
		deps.push_back(dep_line);

	/* Get dependencies of the dependencies of this package, if any */
	size_t deps_size = deps.size();
	for (size_t i = 0; i < deps_size; ++i)
	{
		std::vector<std::string> sub_deps = get_dependencies(deps[i], repo_path);
		deps.insert(deps.end(), sub_deps.begin(), sub_deps.end());
	}

	/* Cache the results */
	dependency_cache[pkg] = deps;

	return deps;
}

std::vector<std::string> deduplicated_dep_list(std::vector<std::string> dependencies)
{
	std::vector<std::string> result;
	std::unordered_map<std::string, bool> exists_in_results;

	/* Don't do anything if there are no dependencies */
	if (dependencies.empty())
		return result;

	/* Start from the end of the list and add each package
	 * to the list only once */
	for (int i = dependencies.size() - 1; i >= 0; --i)
	{
		/* Skip meta-packages */
		if (!meta_packages[dependencies[i]].empty())
			continue;

		/* Check if the package is already in the result list */
		if (exists_in_results[dependencies[i]])
			continue;

		result.push_back(dependencies[i]);
		exists_in_results[dependencies[i]] = true;
	}

	return result;
}

int main(int argc, char** argv)
{
	/* Name of the package that we are inspecting */
	std::string pkg_name;

	/* Location where we can find all of the packages in */
	std::string repo_path = "/var/db/pkg";

	/* Testing args */
	if (argc == 4)
	{
		if (!strcmp(argv[1], "--test"))
		{
			repo_path 		= argv[2];
			pkg_name 		= argv[3];
		}
	}
	else if (argc == 2) /* Only the package name was provided */
	{
		pkg_name = argv[1];
	}
	else
	{
		std::cout << "Incorrect amount of arguments!\n";
		return 1;
	}

	/* Read in the list of meta-packages and parse them into a map */
	std::vector<std::string> meta_package_list = birb::read_file(repo_path + "/meta_packages");
	for (std::string line : meta_package_list)
	{
		/* Find the delimiter */
		size_t pos = line.find(":");

		/* Get the key and the corresponding value and assign it into a map */
		std::vector<std::string> expanded_meta_package = birb::split_string(line.substr(pos + 1, line.size() - (pos + 1)), " ");
		meta_packages[line.substr(0, pos)] = expanded_meta_package;
	}

	/* Get all package dependencies recursively */
	std::vector<std::string> dependencies = get_dependencies(pkg_name, repo_path);

	/* Deduplicate the dependency list */
	dependencies = deduplicated_dep_list(dependencies);

	for (std::string d : dependencies)
		std::cout << d << "\n";

	return 0;
}
