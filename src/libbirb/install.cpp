#include "CLI.hpp"
#include "Database.hpp"
#include "Dependencies.hpp"
#include "Download.hpp"
#include "Install.hpp"
#include "Logging.hpp"
#include "PackageInfo.hpp"
#include "Symlink.hpp"
#include "Utils.hpp"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <unordered_set>

enum class install_phase
{
	setup,
	build,
	install,
	test,
	build32,
	test32,
	install32,
	post_install
};

const static std::unordered_map<install_phase, std::string> install_phase_str = {
	{ install_phase::setup, "_setup" },
	{ install_phase::build, "_build" },
	{ install_phase::install, "_install" },
	{ install_phase::test, "_test" },
	{ install_phase::build32, "_build32" },
	{ install_phase::test32, "_test32" },
	{ install_phase::install32, "_install32" },
	{ install_phase::post_install, "_post_install" }
};

namespace birb
{
	void install(const std::vector<std::string>& packages, const path_settings& paths, const birb_config& config)
	{
		assert(!packages.empty());

		// validate the packages and quit if something seems to be wrong
		for (const std::string& pkg_name : packages)
		{
			if (!validate_package(pkg_name))
				error("Package [", pkg_name, "] does not exist");
		}

		const std::vector<std::string> required_packages = birb::resolve_dependencies(packages);
		assert(!required_packages.empty());

		// figure out which packages have already been installed
		// and what needs to be installed

		const std::vector<std::string> installed_packages_vec = get_installed_packages();
		const std::unordered_set<std::string> installed_packages(installed_packages_vec.begin(), installed_packages_vec.end());

		std::vector<std::string> packages_to_install;
		for (const std::string& pkg_name : required_packages)
			if (!installed_packages.contains(pkg_name))
				packages_to_install.emplace_back(pkg_name);

		if (packages_to_install.empty())
		{
			log("Everything you need is already installed („• ᴗ •„)");
			return;
		}

		std::cout << "The following packages would be installed:\n\n";
		for (const std::string& pkg_name : packages_to_install)
			std::cout << "  " << pkg_name << '\n';

		std::cout << '\n';
		const bool install_confirmed = confirmation_menu("Continue?", true);

		if (!install_confirmed)
			return;

		// read in the package database
		std::vector<std::string> db_file = birb::read_birb_db();

		const bool xorg_is_running = is_process_running("Xorg");

		for (const std::string& pkg_name : packages_to_install)
		{
			log("Starting the installation of package [", pkg_name, "]");

			// do some checks on the package just in case
			if (!is_valid_package_name(pkg_name))
				error("Invalid package name: [", pkg_name, "]");

			const std::optional<pkg_source> repo = locate_package(pkg_name);
			if (!repo.has_value() || !repo.value().is_valid())
				error("Package [", pkg_name, "] does not exists");

			assert(!repo.value().path.empty());

			if (read_pkg_variable(pkg_name, pkg_variable::name, repo.value().path).empty())
				error("Package [", pkg_name, "] does not define a name");

			if (read_pkg_variable(pkg_name, pkg_variable::source, repo.value().path).empty())
				error("Package [", pkg_name, "] does not define a source");

			if (read_pkg_variable(pkg_name, pkg_variable::checksum, repo.value().path).empty())
				error("Package [", pkg_name, "] does not define a checksum");

			// fetch package flags
			const std::unordered_set<pkg_flag> flags = get_pkg_flags(pkg_name, repo.value());

			// start the installation process

			log("Dowloading sources"); // download_package doesn't print this so do it here
			download_package(pkg_name, paths, xorg_is_running);
			install_package(pkg_name, flags, paths, config, xorg_is_running);

			// mark the package as installed

			// if the package is not a dependency, add it into the nest file
			if (std::find(packages.begin(), packages.end(), pkg_name) != packages.end())
			{
				std::ofstream nest_file(paths.nest, std::ios::app);
				if (!nest_file.is_open())
					error("Can't open the nest file for writing");

				nest_file << pkg_name;
			}

			// update the version information in the database
			auto db_entry = std::find_if(db_file.begin(), db_file.end(),
				[&pkg_name](const std::string& entry)
				{
					const std::vector<std::string> entry_tokens = split_string(entry, ";");
					assert(!entry_tokens.empty());

					if (entry_tokens.size() != DB_LINE_COLUMN_COUNT)
						warning("Malformed package database entry: ", entry);

					if (entry_tokens[0] == pkg_name)
						return true;

					return false;
				});

			const std::string version_str = read_pkg_variable(pkg_name, pkg_variable::version, repo.value().path);

			// if no results were found, add a new entry
			if (db_entry == db_file.end())
				db_file.emplace_back(pkg_name + ";" + version_str);

			// if a result was found, update it
			if (db_entry != db_file.end())
				*db_entry = pkg_name + ";" + version_str;
		}

		// write the updated package database to disk
		std::ofstream db_ofstream(BIRB_DB_PATH);
		std::ostream_iterator<std::string> output_iterator(db_ofstream, "\n");
		std::copy(db_file.begin(), db_file.end(), output_iterator);

		if (xorg_is_running)
			set_win_title("done!");

		log("Done!");
	}

	void install_package(const std::string& pkg_name, const std::unordered_set<pkg_flag>& pkg_flags,
			const path_settings& paths, const birb_config& config, const bool xorg_running)
	{
		assert(!pkg_name.empty());
		log("Starting the compiling process");

		// figure out which repository the package is in
		const std::optional<pkg_source> repo = locate_package(pkg_name);
		assert(repo.has_value());

		if (pkg_flags.contains(pkg_flag::wip))
			warning("This package is still considered 'work in progress' and may not be fully functional!");

		if (pkg_flags.contains(pkg_flag::proprietary))
			warning("This package contains binary blobs! Source code may or may not be available. Proceed with caution.");

		// setup the environment variables used by the seed.sh file

		assert(!paths.fakeroot.empty());
		assert(!paths.distfiles.empty());
		assert(!repo.value().path.empty());

		const std::string XORG_PREFIX = std::format("{}/{}/usr", paths.fakeroot, pkg_name);
		const std::string PYTHON_DIST = "usr/python_dist";
		const std::string build_dir_path = std::format("{}/birb_package_build-{}", paths.build_dir, pkg_name);
		info("Build directory: ", build_dir_path);

		assert(!build_dir_path.empty());
		assert(build_dir_path != "/birb_package_build-");

		setenv("PATH", "/usr/local/bin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/python_bin:/opt/rustc/bin", true);
		setenv("PKG_PATH", std::format("{}/{}", repo.value().path, pkg_name).c_str(), true);
		setenv("BUILD_DIR_PATH", paths.distfiles.c_str(), true);
		setenv("DISTFILES", paths.distfiles.c_str(), true);
		setenv("FAKEROOT", paths.fakeroot.c_str(), true);
		setenv("XORG_PREFIX", XORG_PREFIX.c_str(), true);
		setenv("XORG_CONFIG", std::format("--prefix={} --sysconfdir=/etc --localstatedir=/var --disable-static", XORG_PREFIX).c_str(), true);
		setenv("PYTHON_DIST", PYTHON_DIST.c_str(), true);
		setenv("PYTHON_PREFIX", std::format("{}/{}/{}", paths.fakeroot, pkg_name, PYTHON_DIST).c_str(), true);
		setenv("ACLOCAL_PATH", "/usr/share/aclocal", true);
		setenv("XML_CATALOG_FILES", "/etc/xml/catalog", true);
		setenv("GOPATH", "/usr/share/go", true);
		setenv("PKG_CONFIG_PATH", "/usr/lib/pkgconfig:/usr/share/pkgconfig:/usr/lib32/pkgconfig", true);
		setenv("TEMPORARY_BUILD_DIR", build_dir_path.c_str(), true);

		unsetenv("NAME");
		unsetenv("DESC");
		unsetenv("VERSION");
		unsetenv("SOURCE");
		unsetenv("CHECKSUM");
		unsetenv("DEPS");
		unsetenv("FLAGS");
		unsetenv("NOTES");
		unsetenv("_post_install");

		if (std::filesystem::exists(build_dir_path))
			info("Remove the previous build directory at ", build_dir_path);

		info("Creating new build directory to ", build_dir_path);
		std::filesystem::create_directory(build_dir_path);

		// change the current working directory to the build directory
		// since the build scripts expect to for us to be there
		chdir(build_dir_path.c_str());

		log("Setting things up for compiling");
		if (xorg_running)
			set_win_title(std::format("installing {} (setup)", pkg_name));
		const std::string seed_file_path = std::format("{}/{}/seed.sh", paths.repo_dir, pkg_name);
		const std::string pwd_restore_file_path = "/tmp/birb_pwd";
		info("Seed file: ", seed_file_path);

		// remove the old pwd restoring file
		std::filesystem::remove(pwd_restore_file_path);

		const auto exec_seed_phase = [&seed_file_path, &pwd_restore_file_path](const install_phase phase)
		{
			// if the pwd restoring file exists, restore the working directory state
			if (std::filesystem::exists(pwd_restore_file_path))
			{
				std::ifstream file(pwd_restore_file_path);
				if (!file.is_open())
					error("Can't open the pwd restoring file: ", pwd_restore_file_path);

				std::string path;
				file >> path;
				assert(!path.empty());
				chdir(path.c_str());
			}

			exec_shell_cmd(std::format("source {} ; {} ; echo -n $? > /tmp/birb_ret ; pwd > {}", seed_file_path, install_phase_str.at(phase), pwd_restore_file_path));
			const u8 ret = shell_return_value();
			if (ret)
				error("Something went wrong during ", install_phase_str.at(phase), ", ret: ", (i32)ret);
		};

		// call the _setup function in the seed.sh file
		exec_seed_phase(install_phase::setup);

		// TODO: make it possible to customize CFLAGS and CXXFLAGS
		log("Building the package");
		if (xorg_running)
			set_win_title(std::format("installing {} (compile)", pkg_name));
		exec_seed_phase(install_phase::build);

		// run tests if the package has them and test running is enabled
		if (config.enable_tests && pkg_flags.contains(pkg_flag::test))
		{
			log("Running tests");
			if (xorg_running)
				set_win_title(std::format("installing {} (test)", pkg_name));
			exec_seed_phase(install_phase::test);
		}

		log("Installing the package");
		if (xorg_running)
			set_win_title(std::format("installing {} (install)", pkg_name));

		prepare_fakeroot(pkg_name, paths);
		exec_seed_phase(install_phase::install);

		log("Cleaning up");
		if (xorg_running)
			set_win_title(std::format("installing {} (cleanup)", pkg_name));

		std::filesystem::remove_all(build_dir_path);

		log("Symlinking the package fakeroot to the system root");
		if (xorg_running)
			set_win_title(std::format("installing {} (symlink)", pkg_name));

		link_package(pkg_name, paths);
	}

	void prepare_fakeroot(const std::string& pkg_name, const path_settings& paths)
	{
		assert(!pkg_name.empty());

		constexpr static std::array dir_paths = {
			"/usr/lib/bfd-plugins",
			"/usr/lib32",
			"/usr/share/doc",
			"/usr/share/man/man1",
			"/usr/share/man/man2",
			"/usr/share/man/man3",
			"/usr/share/man/man4",
			"/usr/share/man/man5",
			"/usr/share/man/man6",
			"/usr/share/man/man7",
			"/usr/share/man/man8",
			"/usr/share/fonts/TTF",
			"/usr/share/fonts/OTF",
			"/usr/share/applications",
			"/etc/X11/app-defaults",
			"/etc/rc.d/init.d",
			"/etc/rc.d/rc0.d",
			"/etc/rc.d/rc1.d",
			"/etc/rc.d/rc2.d",
			"/etc/rc.d/rc3.d",
			"/etc/rc.d/rc4.d",
			"/etc/rc.d/rc5.d",
			"/etc/rc.d/rc6.d",
			"/etc/rc.d/rcS.d",
			"/usr/bin",
			"/usr/sbin",
			"/usr/include",
			"/etc",
			"/sbin",
		};

		assert(!paths.fakeroot.empty());
		const std::string& fakeroot_path = paths.fakeroot + "/" + pkg_name;
		assert(!fakeroot_path.empty());
		assert(fakeroot_path != "/");

		info("Creating a fakeroot directory");
		for (const std::string dir_path : dir_paths)
			std::filesystem::create_directories(fakeroot_path + "/" + dir_path);
	}
}
