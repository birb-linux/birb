#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#define PKG_SOURCE_CONFIG_PATH "/etc/birb-sources.conf"

struct pkg_source
{
	pkg_source();
	pkg_source(const std::string& name, const std::string& url, const std::string& path);
	std::string name = "";
	std::string url  = "";
	std::string path = "";

	void print();
	bool is_valid();
};

namespace birb
{
	std::vector<pkg_source> get_pkg_sources();

	/* Get the package source repositories in the same ';' separated
	 * format as they are in the configuration file at /etc/birb-sources.conf */
	std::vector<std::string> get_pkg_source_list();

	/* Check if a package exists and if it does, return the source it was found from */
	pkg_source locate_pkg_repo(const std::string& pkg_name, const std::vector<pkg_source>& package_sources);

	std::string read_pkg_variable(const std::string& pkg_name, const std::string& var_name, const std::string& repo_path);

	/* Returns the raw birb_db file if it exists */
	std::vector<std::string> read_birb_db();

	/* Get the list of installed package names with the birb_db file */
	std::vector<std::string> get_installed_packages();

	/* Caching */
	inline std::unordered_map<std::string, std::string> var_cache;
	inline std::unordered_map<std::string, pkg_source> pkg_repo_cache;
	inline std::vector<std::string> installed_packages_cache;
};
