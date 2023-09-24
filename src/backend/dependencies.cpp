#include "Dependencies.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <iostream>
#include <unordered_set>

namespace birb
{
	std::vector<std::string> get_dependencies(const std::string& pkg, const std::vector<pkg_source>& repos, int depth)
	{
		assert(pkg.empty() == false);
		assert(repos.size() > 0);

		/* Check if this package has its dependencies in the cache already */
		if (dependency_cache.contains(pkg))
			return dependency_cache[pkg];

		std::vector<std::string> deps;

		/* Reserve some memory beforehand to avoid too unnecessary std::vector
		 * resizes with packages that have lots of dependencies */
		deps.reserve(100000);

		/* Avoid dependency loops and infinite (or unnecessary) recursion */
		if (depth < 0)
			return deps;

		/* Find the repo that has the package */
		pkg_source repo = birb::locate_pkg_repo(pkg, repos);

		/* Return empty dependency list if the package couldn't be found */
		if (!repo.is_valid())
			return deps;

		/* Perform string splitting only if this isn't a meta package */
		if (!meta_packages.contains(pkg))
		{
			/* Read data from the package file */
			std::string dep_line = birb::read_pkg_variable(pkg, "DEPS", repo.path);

			/* Return empty dependency list if the dep_line is empty
			 * The line could be empty for example when the package doesn't exist or is invalid etc. */
			if (dep_line.empty())
				return deps;

			/* Split the string */
			size_t pos = 0;
			while ((pos = dep_line.find(" ")) != std::string::npos)
			{
				const std::string dep = dep_line.substr(0, pos);

				/* Check if the dependency is a meta package and should be expanded */
				if (meta_packages.contains(dep))
				{
					/* Expand the meta package */
					deps.insert(deps.end(), meta_packages[dep].begin(), meta_packages[dep].end());
				}
				else
				{
					deps.push_back(dep);
				}

				dep_line.erase(0, pos + 1);
			}

			if (!dep_line.empty())
				deps.push_back(dep_line);

			assert(deps.size() > 0);
		}
		else
		{
			deps = meta_packages[pkg];
		}

		/* Get dependencies of the dependencies of this package, if any */
		const size_t deps_size = deps.size();
		for (size_t i = 0; i < deps_size; ++i)
		{
			const std::vector<std::string> sub_deps = get_dependencies(deps[i], repos, depth - 1);
			deps.insert(deps.end(), sub_deps.begin(), sub_deps.end());
		}

		assert(dependency_cache.contains(pkg) == false && "Overwriting old cache results");

		/* Cache the results */
		dependency_cache[pkg] = deps;

		return deps;
	}

	std::vector<std::string> get_reverse_dependencies(const std::string& pkg_name, const std::vector<pkg_source>& repos)
	{
		std::vector<std::string> dependencies;

		/* Get list of installed packages */
		const std::vector<std::string> installed_packages = birb::get_installed_packages();

		/* Get the reverse dependencies for the package with no recursion */
		for (size_t i = 0; i < installed_packages.size(); ++i)
		{
			const std::vector<std::string> temp_deps = get_dependencies(installed_packages[i], repos, 0);

			/* Check if the package had this package we are inspecting in
			 * its dependency list */
			if (std::find(temp_deps.begin(), temp_deps.end(), pkg_name) != temp_deps.end())
				dependencies.push_back(installed_packages[i]);
		}

		return dependencies;
	}

	std::vector<std::string> deduplicated_dep_list(const std::vector<std::string>& dependencies)
	{
		std::vector<std::string> result;

		/* Don't do anything if there are no dependencies */
		if (dependencies.empty())
			return result;

		/* Use the dependency cache to approximately guess the final size of the result vector
		 * so that there wouldn't be any vector resizes */
		result.reserve(dependency_cache.size() * 2);

		std::unordered_set<std::string> exists_in_results;

		/* Start from the end of the list and add each package
		 * to the list only once */
		for (int i = dependencies.size() - 1; i >= 0; --i)
		{
			/* Skip meta-packages */
			if (meta_packages.contains(dependencies[i]))
				continue;

			/* Check if the package is already in the result list */
			if (exists_in_results.contains(dependencies[i]))
				continue;

			result.push_back(dependencies[i]);
			exists_in_results.insert(dependencies[i]);
		}

		return result;
	}

	void orphan_finder(const std::vector<pkg_source>& repos)
	{
		std::unordered_set<std::string> result;

		/* Read in the list of packages installed by the user */
		const std::vector<std::string> nest = birb::read_file("/var/lib/birb/nest");

		/* Get the list of all installed applications */
		const std::vector<std::string> installed_packages = birb::get_installed_packages();

		/* Orphan candidates are packages that are installed but aren't in the nest */
		std::vector<std::string> orphan_candidates;
		orphan_candidates.reserve(installed_packages.size() - nest.size());

		/* Find all orphan candidates */
		for (size_t i = 0; i < installed_packages.size(); ++i)
		{
			if (std::find(nest.begin(), nest.end(), installed_packages[i]) == nest.end())
				orphan_candidates.push_back(installed_packages[i]);
		}
		std::cout << "Found " << orphan_candidates.size() << " orphan candidates\n";

		/* Call it quits if there are no packages that could be orphans */
		if (orphan_candidates.size() == 0)
			return;

		std::cout << "Starting the orphan scan\n";

		pkg_source repo;
		std::string flags_var;
		std::vector<std::string> flags;
		std::vector<std::string> reverse_deps;
		std::vector<std::string> clean_reverse_deps;

		constexpr int max_passes = 256;

		for (int pass = 0; pass < max_passes; ++pass)
		{
			bool clean_run = true;
			for (size_t i = 0; i < orphan_candidates.size(); ++i)
			{
				std::cout << "\rScanning... [Pass: " << pass + 1 << "] [" << i + 1 << "/" << orphan_candidates.size() << "]" << std::flush;

				/* Skip packages that are already in the result list */
				if (result.contains(orphan_candidates[i]))
					continue;

				/* Skip the package if it doesn't have a fakeroot */
				if (!std::filesystem::exists("/var/db/fakeroot/" + orphan_candidates[i]))
					continue;

				/* Check if the package is "important" and should be skipped */
				repo      = birb::locate_pkg_repo(orphan_candidates[i], repos);
				flags_var = birb::read_pkg_variable(orphan_candidates[i], "FLAGS", repo.path);

				if (!flags_var.empty())
				{
					flags = birb::split_string(flags_var, " ");
					if (std::find(flags.begin(), flags.end(), "important") !=  flags.end())
						continue;
				}

				/* Check if the package has any reverse dependencies */
				if (birb::reverse_dependency_cache.contains(orphan_candidates[i]))
				{
					reverse_deps = birb::reverse_dependency_cache[orphan_candidates[i]];
				}
				else
				{
					reverse_deps = birb::get_reverse_dependencies(orphan_candidates[i], repos);
					birb::reverse_dependency_cache[orphan_candidates[i]] = reverse_deps;
				}
				clean_reverse_deps.clear();

				/* Clean out packages that have already been confirmed to be orphans */
				for (size_t j = 0; j < reverse_deps.size(); ++j)
				{
					if (result.contains(reverse_deps[j]))
						continue;

					clean_reverse_deps.push_back(reverse_deps[j]);
				}

				if (clean_reverse_deps.empty())
				{
					result.insert(orphan_candidates[i]);
					clean_run = false;
				}
			}

			/* We found all orphans */
			if (clean_run)
				break;
		}
		std::cout << "\n";

		/* Reverse the result set */
		std::vector<std::string> orphans(result.size());
		int i = orphans.size() - 1;
		for (std::string p : result)
		{
			orphans[i] = p;
			--i;
		}

		for (std::string o : orphans)
			std::cerr << o << " ";

		std::cout << "\n";
	}
}
