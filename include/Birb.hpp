#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#define PKG_SOURCE_CONFIG_PATH "/etc/birb-sources.conf"

struct pkg_source
{
	std::string name;
	std::string url;
	std::string path;

	void print();
	bool is_valid();
};

struct birb
{
	static std::vector<pkg_source> get_pkg_sources();

	/* Get the package source repositories in the same ';' separated
	 * format as they are in the configuration file at /etc/birb-sources.conf */
	static std::vector<std::string> get_pkg_source_list();

	/* Check if a package exists and if it does, return the source it was found from */
	static pkg_source locate_pkg_repo(const std::string& pkg_name, const std::vector<pkg_source>& package_sources);

	static std::vector<std::string> split_string(std::string text, std::string delimiter);
	static std::vector<std::string> read_file(std::string file_path);
	static std::string read_pkg_variable(std::string pkg_name, std::string var_name, std::string repo_path);
	static inline std::unordered_map<std::string, std::string> var_cache;
};
