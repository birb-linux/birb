#include "CLI.hpp"
#include "Download.hpp"
#include "Logging.hpp"
#include "PackageInfo.hpp"
#include "Utils.hpp"

#include <cassert>
#include <format>
#include <fstream>

namespace birb
{
	void download(const std::vector<std::string>& packages, const path_settings& paths)
	{
		assert(!packages.empty());

		// validate the packages and quit if something seems to be wrong
		for (const std::string& pkg_name : packages)
		{
			if (validate_package(pkg_name, paths) != package_validation_error::noerr)
				exit(1);
		}

		std::cout << "The following packages would be downloaded:\n\n";
		for (const std::string& pkg_name : packages)
			std::cout << "  " << pkg_name << '\n';

		std::cout << '\n';

		if (!confirmation_menu("Continue?", true))
			return;

		// check if Xorg is running to avoid unnecessarily setting
		// the window title
		const bool xorg_running = is_process_running("Xorg");

		log("Dowloading sources");
		for (const std::string& pkg_name : packages)
			download_package(pkg_name, paths, xorg_running);
	}

	void download_package(const std::string& pkg_name, const path_settings& paths, const bool xorg_running)
	{
		assert(!pkg_name.empty());

		if (xorg_running)
			set_win_title(std::format("installing {} (download)", pkg_name));

		if (!root_check())
			warning("Downloading source archives to distfiles might not be possible without root privileges (wget will fail silently)");

		// download the source tarball with wget
		// this needs to be done with shell scripting since the seed.sh files might use
		// variables etc. in the source url
		//
		// also this makes using torsocks a bit easier if needed

		assert(!paths.repo_dir.empty());
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

		if (!integrity_file.is_open())
			error("Could not open the file integrity file");

		std::string integrity_check_result;
		integrity_file >> integrity_check_result;
		assert(!integrity_check_result.empty());

		if (integrity_check_result != "ok")
			error("File integrity check failed. Not continuing with the installation");
	}

}
