#pragma once

#include "Config.hpp"

#include <string>
#include <vector>

namespace birb
{
	void link_package(const std::string& pkg_name, const path_settings& paths, const bool force_install);
	void relink_package(const std::vector<std::string>& packages, const path_settings& paths);
	void unlink_package(const std::string& pkg_name, const path_settings& paths);
}
