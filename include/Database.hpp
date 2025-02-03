#pragma once
#include "Config.hpp"
#include <string>
#include <unordered_map>
#include <vector>

constexpr int DB_LINE_COLUMN_COUNT = 2;

struct pkg_source
{
	pkg_source();
	pkg_source(const std::string& name, const std::string& url, const std::string& path);
	std::string name = "";
	std::string url  = "";
	std::string path = "";

	void print() const;
	bool is_valid() const;
};

enum class pkg_variable
{
	name,
	desc,
	version,
	source,
	checksum,
	deps,
	flags,
	notes
};

namespace birb
{
	std::vector<pkg_source> get_pkg_sources(const path_settings& paths);

	/* Get the package source repositories in the same ';' separated
	 * format as they are in the configuration file at /etc/birb-sources.conf */
	std::vector<std::string> get_pkg_source_list(const path_settings& paths);

	/* Check if a package exists and if it does, return the source it was found from */
	pkg_source locate_pkg_repo(const std::string& pkg_name, const std::vector<pkg_source>& package_sources);

	std::string read_pkg_variable(const std::string& pkg_name, const pkg_variable var, const std::string& repo_path);

	/* Returns the raw birb_db file if it exists */
	std::vector<std::string> read_birb_db(const path_settings& paths);

	std::vector<std::string> find_db_entry(const std::vector<std::string>& db_file, const std::string& paths);

	/* Get the list of installed package names with the birb_db file */
	std::vector<std::string> get_installed_packages(const path_settings& paths);

	/* Get versions for all packages taking the repository order into account */
	std::unordered_map<std::string, std::string> get_repo_versions(const path_settings& paths);

	/* Caching */
	inline std::unordered_map<std::string, std::string> var_cache;
	inline std::vector<std::string> installed_packages_cache;
	inline std::unordered_map<std::string, pkg_source> pkg_repo_cache;
}
