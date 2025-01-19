#pragma once

#include "Config.hpp"

namespace birb
{
	// remove packages that were installed as dependencies
	// and have become orphans
	void depclean(const path_settings& paths);
}
