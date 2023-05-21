/**********************************************************/
/* Version management system for the birb package manager */
/**********************************************************/

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string.h>
#include <string>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include "Birb.hpp"


#define BIRB_DB_PATH "/var/lib/birb/birb_db"
#define BIRB_PKG_PATH "/var/db/pkg"
#define BIRB_FAKEROOT_PATH "/var/db/fakeroot"

/* Quit and spit out and error if the command wasn't run
 * as the root user */
void root_check()
{
	if (getuid() != 0)
	{
		std::cout << "This command needs to be run as the root user!\n";
		exit(1);
	}
}

bool argcmp(char* arg, int argc, std::string option, int required_arg_count)
{
	return (!strcmp(arg, option.c_str()) && required_arg_count + 1 < argc);
}

std::vector<std::string> find_db_entry(const std::vector<std::string>& db_file, const std::string& pkg_name)
{
	std::vector<std::string> result(2);

	for (size_t i = 0; i < db_file.size(); ++i)
	{
		/* Split the db line package;version */
		result = birb::split_string(db_file[i], ";");

		if (result[0] == pkg_name)
			return result;
	}

	return std::vector<std::string>(0);
}

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		std::cout << "Missing arguments!\n";
		return -1;
	}

	std::vector<std::string> db_file;

	bool update_db = false;

	if (std::filesystem::exists(BIRB_DB_PATH) && std::filesystem::is_regular_file(BIRB_DB_PATH))
	{
		db_file = birb::read_file(BIRB_DB_PATH);
	}

	if (argcmp(argv[1], argc, "--is-installed", 1))
	{
		std::vector<std::string> db_entry = find_db_entry(db_file, argv[2]);

		if (db_entry.empty())
			std::cout << "no\n";
		else
			std::cout << "yes\n";

		return 0;
	}

	if (argcmp(argv[1], argc, "--list", 0))
	{
		for (std::string i : db_file)
			std::cout << i << "\n";
	}

	if (argcmp(argv[1], argc, "--remove", 1))
	{
		root_check();

		/* Remove the given package from the database */
		db_file.erase(std::remove_if(db_file.begin(), db_file.end(),
					[argv](std::string s)
					{
						return birb::split_string(s, ";")[0] == argv[2];
					}
			), db_file.end());

		update_db = true;
	}

	if (argcmp(argv[1], argc, "--reset", 0))
	{
		root_check();

		/* Clear the db vector */
		db_file.clear();

		/* Get list of all packages in the fakeroot */
		std::vector<std::string> pkgs;
		for (auto& p : std::filesystem::directory_iterator(BIRB_FAKEROOT_PATH))
			if (p.is_directory())
				pkgs.push_back(p.path().filename().string());

		/* Fetch version data and add all of that into the db */
		for (size_t i = 0; i < pkgs.size(); ++i)
		{
			db_file.push_back(pkgs[i] + ";" + birb::read_pkg_variable(pkgs[i], "VERSION", BIRB_PKG_PATH));
		}
		update_db = true;
	}

	if (argcmp(argv[1], argc, "--update", 2))
	{
		root_check();

		/* Attempt to find the entry for this package */
		bool result_found = false;
		std::vector<std::string> db_entry(2);
		for (size_t i = 0; i < db_file.size(); ++i)
		{
			db_entry = birb::split_string(db_file[i], ";");
			if (db_entry[0] == argv[2])
			{
				result_found = true;
				db_file[i] = std::string(argv[2]) + ";" + argv[3];
				break;
			}
		}

		/* Add a new entry if no results were found */
		if (!result_found)
			db_file.push_back(std::string(argv[2]) + ";" + argv[3]);

		update_db = true;
	}

	if (argcmp(argv[1], argc, "--version", 1))
	{
		/* Check the installed version */
		std::vector<std::string> db_entry = find_db_entry(db_file, argv[2]);

		if (db_entry.empty())
			return 1;

		std::cout << db_entry[1] << "\n";
	}

	if (argcmp(argv[1], argc, "--help", 0))
	{
		std::cout << "Options:\n"
			<< "  --is-installed [package]             check if a package is installed\n"
			<< "  --list                               list all installed packages and their versions\n"
			<< "  --remove                             remove a package and its data from the database\n"
			<< "  --reset                              reset the version data with data found from /var/db/pkg\n"
			<< "  --update [package] [new version]     update a package version\n"
			<< "  --version [package]                  get a version of a given package\n";
	}

	/* Write the json_data to disk */
	if (update_db)
	{
		std::ofstream file(BIRB_DB_PATH);
		std::ostream_iterator<std::string> output_iterator(file, "\n");
		std::copy(db_file.begin(), db_file.end(), output_iterator);
		file.close();
	}

	return 0;
}
