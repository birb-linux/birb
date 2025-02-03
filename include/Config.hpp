#pragma once

#include <string>

#include "Logging.hpp"
#include "Types.hpp"

struct path_settings
{
	path_settings()
	{
		// override certain paths if we are installing BirbOS
		// we know that this is the case if the LFS env variable is set
		const char* const env_lfs = getenv("LFS");
		if (env_lfs)
		{
			birb::log("LFS variable is defined (we are probably installing)");
			repo_dir.insert(0, env_lfs);
			db_dir.insert(0, env_lfs);
			build_dir.insert(0, env_lfs);
			fakeroot_backup.insert(0, env_lfs);
			distfiles.insert(0, env_lfs);
			fakeroot.insert(0, env_lfs);
			birb_cfg.insert(0, env_lfs);
			birb_repo_list.insert(0, env_lfs);

			lfs_var_set = true;
			lfs_path = env_lfs;
		}
	}

	std::string repo_dir{"/var/db/pkg"};
	std::string db_dir{"/var/lib/birb"};
	std::string build_dir{"/var/tmp/birb"};
	std::string fakeroot_backup{"/var/backup/birb/fakeroot_backups"};
	std::string distfiles{"/var/cache/distfiles"};
	std::string fakeroot{"/var/db/fakeroot"};
	std::string birb_cfg{"/etc/birb.conf"};
	std::string birb_repo_list{"/etc/birb-sources.conf"};

	std::string nest() const { return db_dir + "/nest"; }
	std::string package_list() const { return db_dir + "/packages"; }
	std::string database() const { return db_dir + "/birb_db"; }
	std::string birb_dist() const { return distfiles + "/birb"; }

	bool lfs_var_set{false};
	std::string lfs_path;
};

struct birb_config
{
	bool enable_lto{true};
	bool enable_tests{false};
	bool enable_32bit_packages{true};
	u16 build_jobs{4};
	std::string birb_remote{"https://github.com/birb-linux/birb"};
};
