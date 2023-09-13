/**********************************************************/
/* Version management system for the birb package manager */
/**********************************************************/

#include "Database.hpp"
#include "Utils.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		std::cout << "Missing arguments!\n";
		return -1;
	}

	/* Locating packages doesn't require reading the database,
	 * so check for that before continuing with the rest of the program */
	if (birb::argcmp(argv[1], argc, "--locate-package", 1))
	{
		std::vector<pkg_source> repo_list = birb::get_pkg_sources();
		pkg_source repo = birb::locate_pkg_repo(argv[2], repo_list);

		if (repo.is_valid())
		{
			/* Package was found, return the repository path */
			std::cout << repo.path << "\n";
			return 0;
		}
		else
		{
			/* Package couldn't be found */
			return 1;
		}
	}

	/* Also this command doesn't need the package database */
	if (birb::argcmp(argv[1], argc, "--list-repositories", 0))
	{
		std::vector<std::string> repository_list = birb::get_pkg_source_list();

		/* Print out the repositories */
		for (std::string r : repository_list)
			std::cout << r << "\n";
	}


	/* If set to true, update the package manager database
	 * at the end of this program */
	bool update_db = false;

	/* Read in the package database */
	std::vector<std::string> db_file = birb::read_birb_db();

	if (birb::argcmp(argv[1], argc, "--diff", 0))
	{
		std::unordered_map<std::string, std::string> repo_data = birb::get_repo_versions();

		/* Go through the installed packages and list out packages with differing versions */
		std::vector<std::string> db_entry(DB_LINE_COLUMN_COUNT);
		for (std::string p : db_file)
		{
			db_entry = birb::split_string(p, ";");

			/* Skip packages that aren't in the pkg repo */
			if (repo_data[db_entry[0]].empty())
				continue;

			/* Compare the versions */
			if (repo_data[db_entry[0]] != db_entry[1])
				std::cout << p << ";" << repo_data[db_entry[0]] << "\n";
		}
	}

	if (birb::argcmp(argv[1], argc, "--is-installed", 1))
	{
		std::vector<std::string> db_entry = birb::find_db_entry(db_file, argv[2]);

		if (db_entry.empty())
			std::cout << "no\n";
		else
			std::cout << "yes\n";

		return 0;
	}

	if (birb::argcmp(argv[1], argc, "--list", 0))
	{
		for (std::string i : db_file)
			std::cout << i << "\n";
	}

	if (birb::argcmp(argv[1], argc, "--remove", 1))
	{
		birb::root_check();

		/* Remove the given package from the database */
		db_file.erase(std::remove_if(db_file.begin(), db_file.end(),
					[argv](std::string s)
					{
						return birb::split_string(s, ";")[0] == argv[2];
					}
			), db_file.end());

		update_db = true;
	}

	if (birb::argcmp(argv[1], argc, "--reset", 0))
	{
		birb::root_check();

		std::cout 	<< "WARNING: Packages that don't have fakeroots won't be counted!\n"
					<< "Do you still want to continue? (y/n): ";
		std::string answer;
		std::cin >> answer;
		if (answer != "Y" && answer != "y")
		{
			std::cout << "Cancelling the reset...\n";
			exit(0);
		}

		std::cout << "Resetting the version database\n";

		/* Clear the db vector */
		db_file.clear();

		/* Get list of all packages in the fakeroot */
		std::unordered_map<std::string, std::string> pkgs = birb::get_repo_versions();

		/* Transform the unordered_map into a std::string vector with the required format */
		std::transform(pkgs.begin(), pkgs.end(), std::back_inserter(db_file), [](const std::pair<std::string, std::string>& pkg) {
			return pkg.first + ";" + pkg.second;
		});

		update_db = true;
	}

	if (birb::argcmp(argv[1], argc, "--update", 2))
	{
		birb::root_check();

		/* Make sure that the package name doesn't use the delimiter char */
		if (std::string(argv[2]).find(";") != std::string::npos)
		{
			std::cout << "The ';' char can't be used in the package name\n";
			exit(3);
		}

		/* Attempt to find the entry for this package */
		bool result_found = false;
		std::vector<std::string> db_entry(DB_LINE_COLUMN_COUNT);
		for (size_t i = 0; i < db_file.size(); ++i)
		{
			db_entry = birb::split_string(db_file[i], ";");

			if (db_entry.size() != DB_LINE_COLUMN_COUNT)
			{
				std::cout << "Possibly malformed database line detected: [" << db_file[i] << "]\n";
				continue;
			}

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

	if (birb::argcmp(argv[1], argc, "--version", 1))
	{
		/* Check the installed version */
		std::vector<std::string> db_entry = birb::find_db_entry(db_file, argv[2]);

		if (db_entry.empty())
			return 1;

		std::cout << db_entry[1] << "\n";
	}

	if (birb::argcmp(argv[1], argc, "--help", 0))
	{
		std::cout << "Options:\n"
			<< "  --diff                               list out-of-date installed packages\n"
			<< "  --is-installed [package]             check if a package is installed\n"
			<< "  --list                               list all installed packages and their versions\n"
			<< "  --list-repositories                  list repositories found from /etc/birb-sources.conf\n"
			<< "  --locate-package [package]           print out the repository that the package can be\n"
			<< "                                       found from, if any\n"
			<< "  --remove                             remove a package and its data from the database\n"
			<< "  --reset                              reset the version data with data found from repositories\n"
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
