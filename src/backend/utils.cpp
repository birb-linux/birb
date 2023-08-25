#include "Utils.hpp"
#include <cassert>
#include <fstream>
#include <iostream>

namespace birb
{
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
