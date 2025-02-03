#pragma once

#include "Config.hpp"

#include <string>
#include <vector>

namespace birb
{
	void download(const std::vector<std::string>& packages, const path_settings& paths);
	void download_package(const std::string& pkg_name, const path_settings& paths, const bool xorg_running);
}
