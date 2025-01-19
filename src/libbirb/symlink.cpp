#include "Symlink.hpp"
#include "Logging.hpp"

#include <filesystem>

namespace birb
{
	void link_package(const std::string& pkg_name, const path_settings& paths)
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
			info(p.string(), " -> ", path_str);

			std::filesystem::create_symlink(p, path_str);
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
