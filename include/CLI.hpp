#pragma once

#include <string>

namespace birb
{
	// ask the user for a confirmation on something
	__attribute__((warn_unused_result))
	bool confirmation_menu(const std::string& msg, const bool default_answer);

	void set_win_title(const std::string& title_text);
}
