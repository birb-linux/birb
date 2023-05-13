/**************************************************/
/* Dependency solver for the birb package manager */
/**************************************************/

#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>

std::vector<std::string> get_dependencies(std::string pkg, std::string repo_path)
{
	std::string pkg_path = repo_path + "/" + pkg + "/seed.sh";
	std::vector<std::string> deps;

	/* Read data from the package file */
	std::string dep_line;
	std::ifstream pkg_file(pkg_path);
	if (pkg_file.is_open())
	{
		while (std::getline(pkg_file, dep_line))
		{
			/* Check if we have located the dependency line */
			if (dep_line.size() > 6 && dep_line.substr(0, 6) == "DEPS=\"")
			{
				/* We are done with the file, stop reading it */
				pkg_file.close();

				/* Break the file reading loop */
				break;
			}
		}
	}
	else
	{
		std::cout << "File [" << pkg_path << "] can't be opened!\n";
		exit(2);
	}

	/* Now that we have located the line that lists all of the dependencies,
	 * we can list them by using whitespace as the delimiter */

	/* First, remove the start and end of the string */
	dep_line.erase(0, 6);
	dep_line.erase(dep_line.size() - 1, 1);

	/* Split the string */
	size_t pos = 0;
	while ((pos = dep_line.find(" ")) != std::string::npos)
	{
		deps.push_back(dep_line.substr(0, pos));
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

	return deps;
}

std::vector<std::string> deduplicated_dep_list(std::vector<std::string> dependencies)
{
	std::vector<std::string> result;

	/* Start from the end of the list and add each package
	 * to the list only once */
	for (size_t i = dependencies.size() - 1; i > 0; --i)
	{
		/* Check if the package is already in the result list */
		bool pkg_found = false;
		for (size_t j = 0; j < result.size(); ++j)
		{
			if (result[j] == dependencies[i])
			{
				pkg_found = true;
				break;
			}
		}

		if (pkg_found)
			continue;

		result.push_back(dependencies[i]);
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

	/* Get all package dependencies recursively */
	std::vector<std::string> dependencies = get_dependencies(pkg_name, repo_path);

	/* Deduplicate the dependency list */
	dependencies = deduplicated_dep_list(dependencies);

	for (std::string d : dependencies)
		std::cout << d << std::endl;

	return 0;
}
