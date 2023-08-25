#include "Birb.hpp"
#include "Utils.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

constexpr char BIRB_DB_PATH[]       = "/var/lib/birb/birb_db";

pkg_source::pkg_source() {}

pkg_source::pkg_source(const std::string& name, const std::string& url, const std::string& path)
:name(name), url(url), path(path)
{}

void pkg_source::print()
{
	std::cout 	<< "Name: \t" 	<< name << "\n"
				<< "URL: \t" 	<< url 	<< "\n"
				<< "Path: \t" 	<< path << "\n";
}

bool pkg_source::is_valid()
{
	return (!name.empty() && !url.empty() && !path.empty());
}

namespace birb
{
	std::vector<pkg_source> get_pkg_sources()
	{
		/* Read the birb-sources.conf file line by line */
		std::vector<std::string> repository_lines = read_file(PKG_SOURCE_CONFIG_PATH);

		std::vector<pkg_source> sources;
		std::vector<std::string> line(3);
		for (size_t i = 0; i < repository_lines.size(); ++i)
		{
			line = split_string(repository_lines[i], ";");

			pkg_source s(line[0], line[1], line[2]);

			assert(s.name.empty() == false);
			assert(s.url.empty()  == false);
			assert(s.path.empty() == false);

			sources.push_back(s);
		}

		return sources;
	}

	std::vector<std::string> get_pkg_source_list()
	{
		return read_file(PKG_SOURCE_CONFIG_PATH);
	}

	pkg_source locate_pkg_repo(const std::string& pkg_name, const std::vector<pkg_source>& package_sources)
	{
		assert(pkg_name.empty() == false);
		assert(package_sources.size() > 0);

		/* Check if the result has already been cached */
		if (pkg_repo_cache.contains(pkg_name))
			return pkg_repo_cache[pkg_name];

		/* Loop through all of the repositories and try to find
		 * the seed.sh file for the given package */
		std::string seed_path;
		for (pkg_source s : package_sources)
		{
			assert(s.path.empty() == false);

			seed_path = s.path + "/" + pkg_name + "/seed.sh";
			if (std::filesystem::exists(seed_path) && std::filesystem::is_regular_file(seed_path))
			{
				///* Cache the results */
				pkg_repo_cache[pkg_name] = s;

				return s;
			}
		}

		return pkg_source("", "", "");
	}

	std::string read_pkg_variable(const std::string& pkg_name, const std::string& var_name, const std::string& repo_path)
	{
		assert(pkg_name.empty() == false);
		assert(var_name.empty() == false);
		assert(repo_path.empty() == false);

		/* Check if the result is already in the cache*/
		std::string key = pkg_name + var_name;
		if (var_cache.contains(key))
			return var_cache[key];

		std::string pkg_path = repo_path + "/" + pkg_name + "/seed.sh";

		/* Read data from the package file */
		std::string var_line;
		std::ifstream pkg_file(pkg_path);

		if (!pkg_file.is_open())
		{
			//std::cout << "File [" << pkg_path << "] can't be opened!\n";
			return "";
		}

		while (std::getline(pkg_file, var_line))
		{
			/* Check if we have located the dependency line */
			if (var_line.substr(0, var_name.size() + 2) == var_name + "=\"")
			{
				/* Break the file reading loop */
				break;
			}
		}

		assert(var_line.size() > var_name.size() + 2 && "The var_line result is too short");

		/* We are done with the file, stop reading it */
		pkg_file.close();

		/* Clean up the string */
		var_line.erase(0, var_name.size() + 2);
		var_line.erase(var_line.size() - 1, 1);

		/* Cache the result */
		var_cache[key] = var_line;

		return var_line;
	}

	std::vector<std::string> read_birb_db()
	{
		std::vector<std::string> db_file;

		/* Read in the package database, if it exists */
		if (std::filesystem::exists(BIRB_DB_PATH) && std::filesystem::is_regular_file(BIRB_DB_PATH))
		{
			db_file = read_file(BIRB_DB_PATH);

			assert(db_file.empty() == false);
		}

		return db_file;
	}

	std::vector<std::string> get_installed_packages()
	{
		/* If the result is already cached, return that instead */
		if (!installed_packages_cache.empty())
			return installed_packages_cache;

		std::vector<std::string> birb_db = read_birb_db();

		/* Split the strings to get package names */
		std::vector<std::string> pkg_names;
		pkg_names.reserve(birb_db.size());
		for (size_t i = 0; i < birb_db.size(); ++i)
			pkg_names.push_back(split_string(birb_db[i], ";")[0]);

		/* Cache the result */
		installed_packages_cache = pkg_names;

		return pkg_names;
	}
}
