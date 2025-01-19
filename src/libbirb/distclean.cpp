#include "Distclean.hpp"
#include "Logging.hpp"

#include <filesystem>

namespace birb
{
	void distclean(const path_settings& paths)
	{
		log("Clearing distcache at ", paths.distfiles);

		size_t total_file_size{0};
		for (std::filesystem::path dist_file : std::filesystem::directory_iterator(paths.distfiles))
		{
			// skip directories (we don't want to remove the birb source code)
			if (!std::filesystem::is_regular_file(dist_file))
				continue;

			total_file_size += std::filesystem::file_size(dist_file);
			std::filesystem::remove(dist_file);
		}

		info("Freed up ", total_file_size / (1024 * 1024), "MB of storage");
		log("Distcache cleared ヽ(*・ω・)ﾉ");
	}
}
