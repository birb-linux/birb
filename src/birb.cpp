#include "Birb.hpp"
#include <fstream>
#include <vector>
#include <iostream>

std::string birb::read_pkg_variable(std::string pkg_name, std::string var_name, std::string repo_path)
{
   	/* Check if the result is already in the cache*/
	if (!var_cache[pkg_name + var_name].empty())
		return var_cache[pkg_name + var_name];

	std::string pkg_path = repo_path + "/" + pkg_name + "/seed.sh";

	/* Read data from the package file */
	std::string var_line;
	std::ifstream pkg_file(pkg_path);
	if (pkg_file.is_open())
	{
		while (std::getline(pkg_file, var_line))
		{
			/* Check if we have located the dependency line */
			if (var_line.size() > var_name.size() + 2 && var_line.substr(0, var_name.size() + 2) == var_name + "=\"")
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

	/* Clean up the string */
	var_line.erase(0, var_name.size() + 2);
	var_line.erase(var_line.size() - 1, 1);

	/* Cache the result */
	var_cache[pkg_name + var_name] = var_line;

	return var_line;
}
