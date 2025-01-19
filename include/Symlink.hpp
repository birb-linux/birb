#pragma once

#include "Config.hpp"

#include <string>

namespace birb
{
	void link_package(const std::string& pkg_name, const path_settings& paths);
	void unlink_package();
}
