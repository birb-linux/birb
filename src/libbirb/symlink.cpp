#include "Symlink.hpp"
#include "Logging.hpp"

#include <cmath>
#include <filesystem>
#include <utility>
#include <vector>

namespace birb
{
	void link_package(const std::string& pkg_name, const path_settings& paths, const bool force_install)
	{
		// fakeroot and root paths gathered from the first try run
		// that checks for conflicts
		// <fakeroot,root>
		std::vector<std::pair<std::string, std::string>> file_paths;

		std::vector<std::string> conflicting_files;

		log("Checking for conflicts");
		const std::string pkg_fakeroot_path = paths.fakeroot + "/" + pkg_name;
		for (const std::filesystem::path& p : std::filesystem::recursive_directory_iterator(pkg_fakeroot_path))
		{
			// skip directories
			if (!std::filesystem::is_regular_file(p))
				continue;

			std::string path_str = p.string();

			// remove the fakeroot path portion from the file path
			path_str.erase(0, pkg_fakeroot_path.size());

			// check for conflict
			if (std::filesystem::exists(path_str))
				conflicting_files.emplace_back(path_str);

			file_paths.emplace_back(std::make_pair(p.string(), path_str));
		}

		if (!conflicting_files.empty() && !force_install)
		{
			non_fatal_error("Conflicting files were found:");
			for (const std::string& file : conflicting_files)
				std::cout << file << '\n';

			log("Deleting the package fakeroot");
			std::filesystem::remove_all(paths.fakeroot + "/" + pkg_name);

			log("Installation cancelled (｡•́︿•̀｡)");

			exit(1);
		}

		if (!conflicting_files.empty() && force_install)
		{
			warning("Deleting conflicting files");
			for (const std::string& file : conflicting_files)
				std::filesystem::remove(file);
		}

		log("Creating symlinks");
		for (const auto& [fakeroot, root] : file_paths)
		{
			info(fakeroot, " -> ", root);
			std::filesystem::create_symlink(fakeroot, root);
		}
	}

	void unlink_package(const std::string& pkg_name, const path_settings& paths)
	{
		const std::string pkg_fakeroot_path = paths.fakeroot + "/" + pkg_name;
		for (const std::filesystem::path& p : std::filesystem::recursive_directory_iterator(pkg_fakeroot_path))
		{
			// skip directories
			if (!std::filesystem::is_regular_file(p))
				continue;

			std::string path_str = p.string();

			// remove the fakeroot path portion from the file path
			path_str.erase(0, pkg_fakeroot_path.size());
			info(path_str);

			std::filesystem::remove(path_str);
		}
	}
}
