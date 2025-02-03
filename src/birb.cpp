#include <cassert>
#include <clipp.h>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <unistd.h>
#include <vector>

#include "Database.hpp"
#include "Depclean.hpp"
#include "Distclean.hpp"
#include "Download.hpp"
#include "Install.hpp"
#include "Logging.hpp"
#include "PackageSearch.hpp"
#include "Symlink.hpp"
#include "Sync.hpp"
#include "Uninstall.hpp"
#include "Utils.hpp"

enum class exec_mode
{
	help,
	download,
	install,
	uninstall,
	depclean,
	distclean,
	relink,
	search,
	sync_repos,
	list_installed,
	update,
	restore,
	upgrade
};

struct opts
{
	exec_mode mode{exec_mode::help};

	// force certain actions through even though it might be risky
	bool force{false};

	// act as if we were running as root
	bool pretend{false};

	std::vector<std::string> packages;
};

int main(int argc, char** argv)
{
	// parse CLI arguments
	opts o;

	auto cli = (
			clipp::option("--pretend").set(o.pretend)
			% "act as if we were running with root privileges",

			clipp::one_of(
				clipp::option("-h", "--help").set(o.mode, exec_mode::help)
				% "display this help page and exit",

				(clipp::option("--download").set(o.mode, exec_mode::download) & clipp::values("package(s)").set(o.packages))
				% "download the source tarball for the given package",

				(clipp::option("-i", "--install").set(o.mode, exec_mode::install)
				 & clipp::option("--force").set(o.force)
				 & clipp::values("package(s)").set(o.packages))
				% "install given package(s) to the filesystem",

				(clipp::option("-u", "--uninstall").set(o.mode, exec_mode::uninstall) & clipp::values("package(s)").set(o.packages))
				% "uninstall given package(s) from the filesystem",

				clipp::option("--depclean").set(o.mode, exec_mode::depclean)
				% "find and uninstall orphan packages",

				clipp::option("--distclean").set(o.mode, exec_mode::distclean)
				% "clear the distcache",

				(clipp::option("--relink").set(o.mode, exec_mode::relink) & clipp::values("package(s)").set(o.packages))
				% "re-create symlinks to the package fakeroots",

				(clipp::option("-s", "--search").set(o.mode, exec_mode::search) & clipp::values("package(s)").set(o.packages))
				% "search for packages by name",

				(clipp::option("--sync").set(o.mode, exec_mode::sync_repos) & clipp::option("--force").set(o.force))
				% "sync package repositories",

				clipp::option("--list-installed").set(o.mode, exec_mode::list_installed)
				% "list all currently installed packages",

				(clipp::option("--restore").set(o.mode, exec_mode::restore) & clipp::value("package").set(o.packages))
				% "restore a fakeroot backup",

				clipp::option("--update").set(o.mode, exec_mode::update)
				% "update out-of-date packages",

				clipp::option("--upgrade").set(o.mode, exec_mode::upgrade)
				% "update the birb package manager"
			) | clipp::values("packages", o.packages).set(o.mode, exec_mode::install) % "install a list of packages"
		);

	if (!clipp::parse(argc, argv, cli) || o.mode == exec_mode::help)
	{
		clipp::doc_formatting fmt;
		fmt.doc_column(40);
		std::cout << clipp::make_man_page(cli, "birb", fmt) << '\n'
			<< "If the argument list only contains package names, birb will attempt downloading and installing them\n";

		if (o.mode != exec_mode::help)
			return 1;

		return 0;
	}

	path_settings path_set;
	birb_config config;

	// override certain paths if we are installing BirbOS
	// we know that this is the case if the LFS env variable is set
	const char* const env_lfs = getenv("LFS");
	if (env_lfs)
	{
		birb::log("LFS variable is defined (we are probably installing)");
		path_set.repo_dir.insert(0, env_lfs);
		path_set.db_dir.insert(0, env_lfs);
		path_set.build_dir.insert(0, env_lfs);
		path_set.fakeroot_backup.insert(0, env_lfs);
		path_set.distfiles.insert(0, env_lfs);
		path_set.fakeroot.insert(0, env_lfs);
		path_set.birb_cfg.insert(0, env_lfs);
		path_set.birb_repo_list.insert(0, env_lfs);
	}

	// verify that the configuration files exist
	if (!std::filesystem::exists(path_set.birb_cfg))
		birb::warning(path_set.birb_cfg, " is missing, please reinstall birb with 'birb --upgrade");

	if (!std::filesystem::exists(path_set.birb_repo_list))
		birb::error(path_set.birb_repo_list, " is missing. Check the TROUBLESHOOTING section in 'man birb' for instructions on how to fix this issue");

	const auto check_root_privileges = [&o]()
	{
		// check if we are running as the root user
		if (!birb::root_check() && !o.pretend)
		{
			birb::error("This command needs to be run with root privileges (￢_￢;) (use the --pretend flag to get around this error if necessary)");
			exit(1);
		}
	};

	switch (o.mode)
	{
		case exec_mode::download:
			check_root_privileges();
			birb::download(o.packages, path_set);
			break;

		case exec_mode::install:
			check_root_privileges();
			birb::install(o.packages, path_set, config, o.force);
			break;

		case exec_mode::uninstall:
			check_root_privileges();
			birb::uninstall(o.packages, path_set);
			break;

		case exec_mode::depclean:
			check_root_privileges();
			birb::depclean(path_set);
			break;

		case exec_mode::distclean:
			check_root_privileges();
			birb::distclean(path_set);
			break;

		case exec_mode::relink:
			check_root_privileges();
			birb::relink_package(o.packages, path_set);
			break;

		case exec_mode::search:
			birb::pkg_search(o.packages, path_set);
			break;

		case exec_mode::sync_repos:
			check_root_privileges();
			birb::sync_repositories(path_set);
			break;

		case exec_mode::list_installed:
		{
			const std::vector<std::string> installed_packages = birb::get_installed_packages(path_set);
			for (const std::string& pkg_name : installed_packages)
				std::cout << pkg_name << '\n';

			break;
		}

		default:
			std::cout << "option not implemented yet\n";
			abort();
	}
}
