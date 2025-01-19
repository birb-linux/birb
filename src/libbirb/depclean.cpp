#include "CLI.hpp"
#include "Database.hpp"
#include "Depclean.hpp"
#include "Dependencies.hpp"
#include "Logging.hpp"
#include "Uninstall.hpp"
#include "Utils.hpp"

#include <string>
#include <vector>

namespace birb
{
	void depclean(const path_settings& paths)
	{
		log("Looking for orphan packages");

		const bool xorg_running = is_process_running("Xorg");
		if (xorg_running)
			set_win_title("finding orphan packages");

		const std::vector<pkg_source> repos = get_pkg_sources();
		const std::vector<std::string> orphan_packages = find_orphan_packages(repos, paths);

		if (orphan_packages.empty())
		{
			log("No orphans were found");
			return;
		}

		info("\nThe following ", orphan_packages.size(), " packages would be uninstalled:\n");
		for (const std::string& pkg_name : orphan_packages)
			std::cout << "  " << pkg_name << '\n';

		if (!confirmation_menu("Continue?", true))
			return;

		log("Uninstalling orphans");
		for (const std::string& pkg_name : orphan_packages)
			uninstall(orphan_packages, paths);

		if (xorg_running)
			set_win_title("done!");
	}
}
