#pragma once

#include <string>
#include <vector>

namespace birb
{
	// find a package and print information about it
	// The output will be in the following format: `package;version;description [installed]`
	void pkg_search(const std::vector<std::string>& packages);
}
