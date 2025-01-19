#pragma once

#include <optional>
#include <string>
#include <vector>

#include "Types.hpp"

namespace birb
{
	__attribute__((warn_unused_result))
	bool root_check();

	void exec_shell_cmd(const std::string& cmd);

	// read the return value gotten from a shell script that gets
	// saved to /tmp/birb_ret
	__attribute__((warn_unused_result))
	u8 shell_return_value();

	// TODO: deprecate and replace with clipp
	__attribute__((warn_unused_result))
	bool argcmp(char* arg, int argc, const std::string& option, int required_arg_count);

	__attribute__((warn_unused_result))
	std::vector<std::string> split_string(std::string text, const std::string& delimiter);

	__attribute__((warn_unused_result))
	std::vector<std::string> read_file(const std::string& file_path);

	// check if a process is running by checking if there is a command running
	// in /proc that has the given process name
	__attribute__((warn_unused_result))
	bool is_process_running(const std::string& process_name);
}
