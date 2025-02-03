#include "Database.hpp"
#include "Logging.hpp"
#include "Sync.hpp"
#include "Utils.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <unistd.h>

namespace birb
{
	void sync_repositories(const path_settings& paths)
	{
		log("Syncing package repositories");

		// get list of the repos
		std::vector<pkg_source> repos = get_pkg_sources(paths);

		// remove the old package list
		std::filesystem::remove(paths.package_list());

		// new list for the package list cache
		std::vector<std::string> package_name_list;

		// sync each repo
		for (const pkg_source& repo : repos)
		{
			log("Syncing ", repo.name);

			if (!std::filesystem::exists(repo.path))
			{
				warning("The repo ", repo.name, " was missing. Cloning it...");
				exec_shell_cmd(std::format("git clone {} {}", repo.url, repo.path));
			}
			else
			{
				std::filesystem::current_path(repo.path);
				info("Repo path: ", std::filesystem::current_path());
				exec_shell_cmd("git fetch ; git pull");
			}

			// cache the package list
			for (std::filesystem::path p : std::filesystem::directory_iterator(repo.path))
			{
				// skip the birb source code
				if (p.filename() == "birb")
					continue;

				package_name_list.emplace_back(p.filename());
			}
		}

		// write the new package list to disk
		std::ofstream new_pkg_list(paths.package_list(), std::ios_base::app);
		for (const std::string& pkg_name : package_name_list)
			new_pkg_list << pkg_name << '\n';
	}
}
