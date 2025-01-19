#pragma once

#include <iostream>

namespace birb
{
	template<typename... Args>
	void info(Args... args)
	{
		(std::cout << ... << args) << '\n';
	}

	template<typename... Args>
	void log(Args... args)
	{
		std::cout << "\033[32m=> \033[0m";
		(std::cout << ... << args) << '\n';
	}

	template<typename... Args>
	void warning(Args... args)
	{
		std::cout << "\033[35mWARNING: ";
		(std::cout << ... << args);
		std::cout << "\033[0m\n";
	}

	template<typename... Args>
	void error(Args... args)
	{
		std::cerr << "\033[31mERROR: ";
		(std::cerr << ... << args);
		std::cout << "\033[0m\n";
		exit(1);
	}
}
