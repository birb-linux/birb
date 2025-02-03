#pragma once

#include <string>

#include "Types.hpp"

struct path_settings
{
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
};

struct birb_config
{
	bool enable_lto{true};
	bool enable_tests{false};
	bool enable_32bit_packages{true};
	u16 build_jobs{4};
	std::string birb_remote{"https://github.com/birb-linux/birb"};
};
