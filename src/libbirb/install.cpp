#include "CLI.hpp"
#include "Database.hpp"
#include "Dependencies.hpp"
#include "Install.hpp"
#include "Logging.hpp"
#include "PackageInfo.hpp"
#include "Utils.hpp"

#include <fstream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <unordered_set>

namespace birb
{
	void install(const std::vector<std::string>& packages, const path_settings& paths, const birb_config& config)
	{
		// validate the packages and quit if something seems to be wrong
		for (const std::string& pkg_name : packages)
		{
			if (!validate_package(pkg_name))
				error("Package [", pkg_name, "] does not exist");
		}

		const std::vector<std::string> required_packages = birb::resolve_dependencies(packages);

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
		const bool install_confirmed = confirmation_menu("Continue", true);

		if (!install_confirmed)
			return;

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

			if (read_pkg_variable(pkg_name, "NAME", repo.value().path).empty())
				error("Package [", pkg_name, "] does not define a name");

			if (read_pkg_variable(pkg_name, "SOURCE", repo.value().path).empty())
				error("Package [", pkg_name, "] does not define a source");

			if (read_pkg_variable(pkg_name, "CHECKSUM", repo.value().path).empty())
				error("Package [", pkg_name, "] does not define a checksum");

			// fetch package flags
			const std::unordered_set<pkg_flag> flags = get_pkg_flags(pkg_name, repo.value());

			// start the installation process

			if (xorg_is_running) set_win_title(std::format("installing {} (download)", pkg_name));
			download_package(pkg_name, paths);

			if (xorg_is_running) set_win_title(std::format("installing {} (compile)", pkg_name));
			compile_package(pkg_name, flags, paths, config);

			if (xorg_is_running) set_win_title(std::format("installing {} (install)", pkg_name));
			install_package(pkg_name, flags, paths, config);
		}

		if (xorg_is_running)
			set_win_title("done!");
	}

	void download_package(const std::string& pkg_name, const path_settings& paths)
	{
		if (!root_check())
			warning("Downloading source archives to distfiles might not be possible without root privileges (wget will fail silently)");

		// download the source tarball with wget
		// this needs to be done with shell scripting since the seed.sh files might use
		// variables etc. in the source url
		//
		// also this makes using torsocks a bit easier if needed

		const std::string seed_file_path = std::format("{}/{}/seed.sh", paths.repo_dir, pkg_name);

		const std::string download_script = std::format(R"~~(
set -e

# source the seed.sh file
source {}

# get the tarball name
TARBALL="$(basename $SOURCE)"

# check if the tarball has already been downloaded
DISTFILES={}
if [ -f "$DISTFILES/$TARBALL" ]
then
	echo -n "$TARBALL found in distcache, comparing checksums... "
	CACHE_CHECKSUM="$(md5sum $DISTFILES/$TARBALL | cut -d ' ' -f1)"

	if [ "$CACHE_CHECKSUM" == "$CHECKSUM" ]
	then
		echo "ok"
		echo -n "ok" > /tmp/birb_integrity_check
		exit 0
	fi
fi

echo "Fetching $TARBALL..."
wget -q --show-progress --directory-prefix="$DISTFILES" "$SOURCE"

echo -n "Verifying integrity... "
CACHE_CHECKSUM="$(md5sum $DISTFILES/$TARBALL | cut -d ' ' -f1)"

if [ "$CACHE_CHECKSUM" == "$CHECKSUM" ]
then
	echo "ok"
	echo -n "ok" > /tmp/birb_integrity_check
	exit 0
fi

echo "fail"
echo -n "fail" > /tmp/birb_integrity_check
)~~", seed_file_path, paths.distfiles);

		exec_shell_cmd(download_script);

		// check the integrity file to see if the download went okay
		std::ifstream integrity_file("/tmp/birb_integrity_check");
		std::string integrity_check_result;
		integrity_file >> integrity_check_result;

		if (integrity_check_result != "ok")
			error("File integrity check failed. Not continuing with the installation");
	}

	void compile_package(const std::string& pkg_name, const std::unordered_set<pkg_flag>& pkg_flags,
			const path_settings& paths, const birb_config& config)
	{
		// figure out which repository the package is in
		const std::optional<pkg_source> repo = locate_package(pkg_name);
		assert(repo.has_value());

		if (pkg_flags.contains(pkg_flag::wip))
			warning("This package is still considered 'work in progress' and may not be fully functional!");

		if (pkg_flags.contains(pkg_flag::proprietary))
			warning("This package contains binary blobs! Source code may or may not be available. Proceed with caution.");

		// setup the environment variables used by the seed.sh file

		const std::string XORG_PREFIX = std::format("{}/{}/usr", paths.fakeroot, pkg_name);

		setenv("PATH", "/usr/local/bin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/python_bin:/opt/rustc/bin", true);
		setenv("PKG_PATH", std::format("{}/{}", repo->path, pkg_name).c_str(), true);
		setenv("XORG_PREFIX", XORG_PREFIX.c_str(), true);
		setenv("XORG_CONFIG", std::format("--prefix={} --sysconfdir=/etc --localstatedir=/var --disable-static", XORG_PREFIX).c_str(), true);
		setenv("PYTHON_DIST", "usr/python_dist", true);
		setenv("ACLOCAL_PATH", "/usr/share/aclocal", true);
	}

	void install_package(const std::string& pkg_name, const std::unordered_set<pkg_flag>& pkg_flags,
			const path_settings& paths, const birb_config& config)
	{
	}
}
