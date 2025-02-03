#pragma once

#include "Config.hpp"

#include <string>
#include <vector>

namespace birb
{
	void uninstall(const std::vector<std::string>& packages, const path_settings& paths);
}
