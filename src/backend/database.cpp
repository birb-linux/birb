#include "Database.hpp"
#include "Utils.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>

pkg_source::pkg_source() {}

pkg_source::pkg_source(const std::string& name, const std::string& url, const std::string& path)
:name(name), url(url), path(path)
{}

void pkg_source::print() const
{
	std::cout 	<< "Name: \t" 	<< name << "\n"
				<< "URL: \t" 	<< url 	<< "\n"
				<< "Path: \t" 	<< path << "\n";
}

bool pkg_source::is_valid() const
{
	return (!name.empty() && !url.empty() && !path.empty());
}

namespace birb
{
	std::vector<pkg_source> get_pkg_sources()
	{
		/* Read the birb-sources.conf file line by line */
		const std::vector<std::string> repository_lines = read_file(PKG_SOURCE_CONFIG_PATH);

		std::vector<pkg_source> sources;
		for (size_t i = 0; i < repository_lines.size(); ++i)
		{
			std::vector<std::string> line = split_string(repository_lines[i], ";");

			pkg_source s(line.at(0), line.at(1), line.at(2));

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
		for (pkg_source s : package_sources)
		{
			assert(s.path.empty() == false);

			const std::string seed_path = s.path + "/" + pkg_name + "/seed.sh";
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
		const std::string key = pkg_name + var_name;
		if (var_cache.contains(key))
			return var_cache[key];

		const std::string pkg_path = repo_path + "/" + pkg_name + "/seed.sh";

		/* Read data from the package file */
		std::string var_line;
		std::ifstream pkg_file(pkg_path);

		if (!pkg_file.is_open())
		{
			//std::cout << "File [" << pkg_path << "] can't be opened!\n";
			return "";
		}

		const std::string var_line_beginning = var_name + "=\"";
		while (std::getline(pkg_file, var_line))
		{
			/* Check if we have located the dependency line */
			if (var_line.substr(0, var_name.size() + 2) == var_line_beginning)
			{
				/* Break the file reading loop */
				break;
			}
		}

		if (var_line.size() <= var_name.size() + 2)
		{
			std::cerr << "Package " << pkg_name << " is corrupted! Please check the formatting for variable '" << var_name << "' in " << repo_path << "/" << pkg_name << "/seed.sh\n";
			exit(1);
		}

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

	std::vector<std::string> find_db_entry(const std::vector<std::string>& db_file, const std::string& pkg_name)
	{
		assert(db_file.empty() == false);
		assert(pkg_name.empty() == false);

		for (size_t i = 0; i < db_file.size(); ++i)
		{
			/* Split the db line package;version */
			std::vector<std::string> result = birb::split_string(db_file[i], ";");

			if (result.at(0) == pkg_name && result.size() == DB_LINE_COLUMN_COUNT)
				return result;
		}

		return std::vector<std::string>(0);
	}

	std::vector<std::string> get_installed_packages()
	{
		/* If the result is already cached, return that instead */
		if (!installed_packages_cache.empty())
			return installed_packages_cache;

		const std::vector<std::string> birb_db = read_birb_db();

		/* Split the strings to get package names */
		std::vector<std::string> pkg_names;
		pkg_names.reserve(birb_db.size());
		for (size_t i = 0; i < birb_db.size(); ++i)
			pkg_names.push_back(split_string(birb_db[i], ";")[0]);

		/* Cache the result */
		installed_packages_cache = pkg_names;

		return pkg_names;
	}

	std::unordered_map<std::string, std::string> get_repo_versions()
	{
		/* Repository list */
		const std::vector<pkg_source> pkg_sources = birb::get_pkg_sources();
		assert(pkg_sources.size() > 0 && "Package sources couldn't be found");

		std::unordered_map<std::string, std::string> pkgs;

		/* Get list of all packages in the fakeroot */
		for (auto& p : std::filesystem::directory_iterator(BIRB_FAKEROOT_PATH))
		{
			if (p.is_directory())
			{
				/* Iterate through the package source repositories and get the version
				 * from the first repository that has the package in it */
				pkgs[p.path().filename().string()] = birb::read_pkg_variable(p.path().filename().string(), "VERSION", BIRB_PKG_PATH);
			}
		}

		return pkgs;
	}
}
