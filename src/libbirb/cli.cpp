#include "CLI.hpp"

#include <cassert>
#include <ios>
#include <iostream>
#include <limits>

namespace birb
{
	bool confirmation_menu(const std::string& msg, const bool default_answer)
	{
		assert(!msg.empty());
		std::cout << msg << " (" << (default_answer ? 'Y' : 'y') << "/" << (!default_answer ? 'N' : 'n') << "): ";

		while (true)
		{
			const char answer = std::tolower(std::cin.get());

			if (answer == '\n')
				return default_answer;

			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			if (answer == 'y')
				return true;

			if (answer == 'n')
				return false;

			std::cout << "invalid answer, try again: ";
		}
	}

	void set_win_title(const std::string& title_text)
	{
		assert(!title_text.empty());
		std::cout << "\033kbirb: " << title_text << "\033\\";
	}
}
