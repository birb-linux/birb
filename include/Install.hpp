#pragma once

#include <string>
#include <unordered_set>
#include <vector>

#include "Config.hpp"
#include "PackageInfo.hpp"

namespace birb
{
	// start the process of installing packages to the system
	void install(const std::vector<std::string>& packages, const path_settings& paths, const birb_config& config);

	void download_package(const std::string& pkg_name, const path_settings& paths, const bool xorg_running);

	void install_package(const std::string& pkg_name, const std::unordered_set<pkg_flag>& pkg_flags,
			const path_settings& paths, const birb_config& config, const bool xorg_running);

	// create an empty skeleton fakeroot for a papckage
	void prepare_fakeroot(const std::string& pkg_name, const path_settings& paths);
}
