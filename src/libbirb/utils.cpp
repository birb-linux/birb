#include <cstdlib>
#ifdef BIRB_TEST
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#endif /* BIRB_TEST */

#include "Logging.hpp"
#include "Utils.hpp"

#include <cassert>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <vector>

namespace birb
{
	bool root_check()
	{
		return getuid() == 0;
	}

	void exec_shell_cmd(const std::string& cmd)
	{
		assert(!cmd.empty());

		FILE* const bash_pipe = popen("bash", "w");
		if (!bash_pipe)
			error("Can't open a pipe to bash");

		fputs(cmd.c_str(), bash_pipe);
		pclose(bash_pipe);
	}

	u8 shell_return_value()
	{
		std::ifstream file("/tmp/birb_ret");
		if (!file.is_open())
			error("Could not open /tmp/birb_ret");

		std::string ret_str;
		file >> ret_str;
		return std::atoi(ret_str.c_str());
	}

	bool argcmp(char* arg, int argc, const std::string& option, int required_arg_count)
	{
		assert(arg != NULL);
		assert(option.empty() == false);
		assert(required_arg_count >= 0);

		return (!strcmp(arg, option.c_str()) && required_arg_count + 1 < argc);
	}

	std::vector<std::string> split_string(std::string text, const std::string& delimiter)
	{
		assert(text.empty() == false);
		assert(delimiter.empty() == false);

		std::vector<std::string> result;

		/* Split the string */
		size_t pos = 0;
		while ((pos = text.find(delimiter)) != std::string::npos)
		{
			result.push_back(text.substr(0, pos));
			text.erase(0, pos + delimiter.length());
		}

		if (!text.empty())
			result.push_back(text);

		return result;
	}

#ifdef BIRB_TEST

/* Make cppcheck happy */
#ifndef TEST_CASE
#define TEST_CASE
#define SUBCASE
#endif

	TEST_CASE("split_string()")
	{
		SUBCASE("One char delimiter")
		{
			std::vector<std::string> A = split_string("The quick brown fox jumps over the lazy dog", " ");
			CHECK(A.size() == 9);
			CHECK(A[0] == "The");
			CHECK(A[1] == "quick");
			CHECK(A[2] == "brown");
			CHECK(A[3] == "fox");
			CHECK(A[4] == "jumps");
			CHECK(A[5] == "over");
			CHECK(A[6] == "the");
			CHECK(A[7] == "lazy");
			CHECK(A[8] == "dog");
		}

		SUBCASE("One char delimiter with no splits")
		{
			std::vector<std::string> A = split_string("Hello_world!", " ");
			CHECK(A.size() == 1);
			CHECK(A[0] == "Hello_world!");
		}

		SUBCASE("Multichar delimiter")
		{
			std::vector<std::string> A = split_string("HelloASDworld!", "ASD");
			CHECK(A.size() == 2);
			CHECK(A[0] == "Hello");
			CHECK(A[1] == "world!");
		}
	}
#endif

	std::vector<std::string> read_file(const std::string& file_path)
	{
		assert(file_path.empty() == false);

		std::ifstream file(file_path);

		if (!file.is_open())
		{
			std::cout << "File [" << file_path << "] can't be opened!\n";
			exit(2);
		}

		std::string line;
		std::vector<std::string> lines;

		/* Read the file */
		while (std::getline(file, line))
		{
			/* Ignore empty lines and lines starting with '#' */
			if (line.size() == 0 || line.at(0) == '#')
				continue;

			lines.push_back(line);
		}

		file.close();

		return lines;
	}

	bool is_process_running(const std::string& process_name)
	{
		assert(!process_name.empty());
		for (const auto& dir : std::filesystem::directory_iterator("/proc"))
		{
			// read the command
			std::ifstream file(std::format("{}/comm", dir.path().string()));

			// if the file could not be opened for reading, the directory
			// is probably for something other than a process PID
			if (!file.is_open())
				continue;

			std::string command;
			file >> command;

			if (command == process_name)
				return true;
		}

		return false;
	}
}
