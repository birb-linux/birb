#pragma once

#include "Config.hpp"

#include <string>

namespace birb
{
	void link_package(const std::string& pkg_name, const path_settings& paths, const bool force_install);
	void unlink_package(const std::string& pkg_name, const path_settings& paths);
}
