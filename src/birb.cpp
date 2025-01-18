#include <clipp.h>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#include "Logging.hpp"

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

struct path_settings
{
	std::string repo_dir{"/var/db/pkg"};
	std::string db_dir{"/var/lib/birb"};
	std::string nest{db_dir + "/nest"};
	std::string package_list{db_dir + "/packages"};
	std::string build_dir{"/var/tmp/birb"};
	std::string fakeroot_backup{"/var/backup/birb/fakeroot_backups"};
	std::string distfiles{"/var/cache/distfiles"};
	std::string fakeroot{"/var/db/fakeroot"};
	std::string birb_dist{distfiles + "/birb"};
	std::string birb_cfg{"/etc/birb.conf"};
	std::string birb_repo_list{"/etc/birb-sources.conf"};
};

struct opts
{
	exec_mode mode;
	bool force;
	std::vector<std::string> packages;
};

int main(int argc, char** argv)
{
	// parse CLI arguments
	opts o;

	auto cli = (
			clipp::one_of(
				clipp::option("-h", "--help").set(o.mode, exec_mode::help)
				% "display this help page and exit",

				(clipp::option("--download").set(o.mode, exec_mode::download) & clipp::values("package(s)").set(o.packages))
				% "download the source tarball for the given package",

				(clipp::option("-i", "--install").set(o.mode, exec_mode::install) & clipp::values("package(s)").set(o.packages))
				% "install given package(s) to the filesystem",

				(clipp::option("-u", "--uninstall").set(o.mode, exec_mode::uninstall) & clipp::values("package(s)").set(o.packages))
				% "uninstall given package(s) from the filesystem",

				clipp::option("--depclean").set(o.mode, exec_mode::depclean)
				% "find and uninstall orphan packages",

				clipp::option("--distclean").set(o.mode, exec_mode::distclean)
				% "clear the distcache",

				(clipp::option("--relink").set(o.mode, exec_mode::relink) & clipp::values("package(s)").set(o.packages))
				% "re-create symlinks to the package fakeroots",

				(clipp::option("-s", "--search").set(o.mode, exec_mode::search) & clipp::value("package").set(o.packages))
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
			) |
			clipp::values("packages", o.packages).set(o.mode, exec_mode::install) % "install a list of packages"
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

	// override certain paths if we are installing BirbOS
	// we know that this is the case if the LFS env variable is set
	const char* const env_lfs = getenv("LFS");
	if (env_lfs)
	{
		log("LFS variable is defined (we are probably installing)");
		path_set.repo_dir.insert(0, env_lfs);
		path_set.distfiles.insert(0, env_lfs);
		path_set.fakeroot.insert(0, env_lfs);
		path_set.birb_cfg.insert(0, env_lfs);
		path_set.birb_repo_list.insert(0, env_lfs);
	}

	// verify that the configuration files exist
	if (!std::filesystem::exists(path_set.birb_cfg))
		warning(path_set.birb_cfg, " is missing, please reinstall birb with 'birb --upgrade");

	if (!std::filesystem::exists(path_set.birb_repo_list))
	{
		error(path_set.birb_repo_list, " is missing. Check the TROUBLESHOOTING section in 'man birb' for instructions on how to fix this issue");
		return 1;
	}

	switch (o.mode)
	{
		default:
			std::cout << "option not implemented yet\n";
			abort();
	}
}
