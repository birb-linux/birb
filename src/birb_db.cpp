/**********************************************************/
/* Version management system for the birb package manager */
/**********************************************************/

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>
#include "Birb.hpp"
#include "json.hpp"


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

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		std::cout << "Missing arguments!\n";
		return -1;
	}

	nlohmann::json json_data;
	bool update_db = false;

	if (std::filesystem::exists(BIRB_DB_PATH) && std::filesystem::is_regular_file(BIRB_DB_PATH))
	{
		std::ostringstream str_stream;
		std::ifstream file(BIRB_DB_PATH);
		str_stream << file.rdbuf();

		json_data = nlohmann::json::parse(str_stream.str());
		file.close();
	}

	if (argcmp(argv[1], argc, "--is-installed", 1))
	{
		if (!json_data["packages"][argv[2]].empty())
			std::cout << "yes\n";
		else
			std::cout << "no\n";
	}

	if (argcmp(argv[1], argc, "--list", 0))
	{
		for (auto& element : json_data["packages"])
		{
			std::cout << std::string(element["name"]) << ";" << std::string(element["version"]) << std::endl;
		}
	}

	if (argcmp(argv[1], argc, "--remove", 1))
	{
		root_check();

		/* Remove the given package from the database */
		json_data["packages"].erase(argv[2]);
		update_db = true;
	}

	if (argcmp(argv[1], argc, "--reset", 0))
	{
		root_check();

		/* Clear the json data */
		json_data.clear();

		/* Get list of all packages in the fakeroot */
		std::vector<std::string> pkgs;
		for (auto& p : std::filesystem::directory_iterator(BIRB_FAKEROOT_PATH))
			if (p.is_directory())
				pkgs.push_back(p.path().filename().string());

		/* Fetch version data and add all of that into the db */
		for (size_t i = 0; i < pkgs.size(); ++i)
		{
			json_data["packages"][pkgs[i]]["name"] = pkgs[i];
			json_data["packages"][pkgs[i]]["version"] = birb::read_pkg_variable(pkgs[i], "VERSION", BIRB_PKG_PATH);
		}
		update_db = true;
	}

	if (argcmp(argv[1], argc, "--update", 2))
	{
		root_check();

		/* Create an entry for the package and its version */
		json_data["packages"][argv[2]]["name"] = argv[2];
		json_data["packages"][argv[2]]["version"] = argv[3];
		update_db = true;
	}

	if (argcmp(argv[1], argc, "--version", 1))
	{
		/* Check the installed version */
		if (!json_data["packages"][argv[2]].empty())
			std::cout << std::string(json_data["packages"][argv[2]]["version"]) << std::endl;
		else
			std::cout << "Package [" << argv[2] << "] is not installed!" << std::endl;
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
		file << std::setw(4) << json_data << std::endl;
		file.close();
	}

	return 0;
}
