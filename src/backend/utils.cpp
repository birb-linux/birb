#include <doctest/doctest.h>
#include "Utils.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <vector>

namespace birb
{
	/* Quit and spit out and error if the command wasn't run
	 * as the root user */
	void root_check()
	{
		if (getuid() != 0)
		{
			std::cout << "This command needs to be run as the root user!\n";
			exit(1);
		}
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

	std::vector<std::string> read_file(const std::string& file_path)
	{
		assert(file_path.empty() == false);

		std::ifstream file(file_path);

		if (!file.is_open())
		{
			std::cout << "File [" << file_path << " can't be opened!\n";
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
}
