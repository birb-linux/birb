/**************************************************/
/* Dependency solver for the birb package manager */
/**************************************************/

#include "Birb.hpp"
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

/* Function declarations */
std::vector<std::string> get_dependencies(const std::string& pkg, const std::vector<pkg_source>& repos, int depth);
std::vector<std::string> deduplicated_dep_list(const std::vector<std::string>& dependencies);


static std::unordered_map<std::string, std::vector<std::string>> meta_packages;
static std::unordered_map<std::string, std::vector<std::string>> dependency_cache;


std::vector<std::string> get_dependencies(const std::string& pkg, const std::vector<pkg_source>& repos, int depth)
{
	assert(pkg.empty() == false);
	assert(repos.size() > 0);

	/* Check if this package has its dependencies in the cache already */
	if (dependency_cache.contains(pkg))
		return dependency_cache[pkg];

	std::vector<std::string> deps;

	/* Avoid dependency loops and infinite (or unnecessary) recursion */
	if (depth < 0)
		return deps;

	/* Find the repo that has the package */
	pkg_source repo = birb::locate_pkg_repo(pkg, repos);

	/* Return empty dependency list if the package couldn't be found */
	if (!repo.is_valid())
		return deps;

	/* Perform string splitting only if this isn't a meta package */
	if (!meta_packages.contains(pkg))
	{
		/* Read data from the package file */
		std::string dep_line = birb::read_pkg_variable(pkg, "DEPS", repo.path);

		/* Return empty dependency list if the dep_line is empty
		 * The line could be empty for example when the package doesn't exist or is invalid etc. */
		if (dep_line.empty())
			return deps;

		/* Split the string */
		size_t pos = 0;
		while ((pos = dep_line.find(" ")) != std::string::npos)
		{
			std::string dep = dep_line.substr(0, pos);

			/* Check if the dependency is a meta package and should be expanded */
			if (meta_packages.contains(dep))
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

		assert(deps.size() > 0);
	}
	else
	{
		deps = meta_packages[pkg];
	}

	/* Get dependencies of the dependencies of this package, if any */
	size_t deps_size = deps.size();
	for (size_t i = 0; i < deps_size; ++i)
	{
		std::vector<std::string> sub_deps = get_dependencies(deps[i], repos, depth - 1);
		deps.insert(deps.end(), sub_deps.begin(), sub_deps.end());
	}

	assert(dependency_cache.contains(pkg) == false && "Overwriting old cache results");

	/* Cache the results */
	dependency_cache[pkg] = deps;

	return deps;
}

std::vector<std::string> deduplicated_dep_list(const std::vector<std::string>& dependencies)
{
	std::vector<std::string> result;

	/* Don't do anything if there are no dependencies */
	if (dependencies.empty())
		return result;

	std::unordered_set<std::string> exists_in_results;

	/* Start from the end of the list and add each package
	 * to the list only once */
	for (int i = dependencies.size() - 1; i >= 0; --i)
	{
		/* Skip meta-packages */
		if (meta_packages.contains(dependencies[i]))
			continue;

		/* Check if the package is already in the result list */
		if (exists_in_results.contains(dependencies[i]))
			continue;

		result.push_back(dependencies[i]);
		exists_in_results.insert(dependencies[i]);
	}

	return result;
}

int main(int argc, char** argv)
{
	/* Name of the package that we are inspecting */
	std::string pkg_name;

	/* Location where we can find all of the packages in */
	std::vector<pkg_source> repos;

	/* Dependency resolution direction */
	bool resolve_reverse = false;

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
			resolve_reverse = true;
		}
		else /* No valid argument was given */
		{
			std::cout << "Invalid argument: " << argv[1] << "\n";
			exit(1);
		}

		repos = birb::get_pkg_sources();
		pkg_name = argv[2];
		repos = birb::get_pkg_sources();
	}
	else if (argc == 2) /* Only the package name was provided */
	{
		pkg_name = argv[1];
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
		meta_packages[line.substr(0, pos)] = birb::split_string(line.substr(pos + 1, line.size() - (pos + 1)), " ");
	}

	std::vector<std::string> dependencies;

	if (!resolve_reverse) /* Resolve dependencie normally for the given package */
	{
		/* Get all package dependencies recursively */
		dependencies = get_dependencies(pkg_name, repos, 512);

		/* Deduplicate the dependency list */
		dependencies = deduplicated_dep_list(dependencies);
	}
	else /* Find packages that depend on this given package */
	{
		/* Get list of installed packages */
		std::vector<std::string> installed_packages = birb::get_installed_packages();

		/* Get the reverse dependencies for the package with no recursion */
		std::vector<std::string> temp_deps;
		for (size_t i = 0; i < installed_packages.size(); ++i)
		{
			temp_deps = get_dependencies(installed_packages[i], repos, 0);

			/* Check if the package had this package we are inspecting in
			 * its dependency list */
			if (std::find(temp_deps.begin(), temp_deps.end(), pkg_name) != temp_deps.end())
				dependencies.push_back(installed_packages[i]);
		}
	}

	for (std::string d : dependencies)
		std::cout << d << "\n";

	return 0;
}
