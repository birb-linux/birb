#ifdef BIRB_TEST
#define DDOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

/* This file is empty on puporse
 *
 * It is simply an entry for doctest to implement its
 * main() function. The tests will be implemented below
 * the functions that they are testing
 *
 * The test code will be removed from the final binaries during linking */

int main(int argc, char** argv)
{
	doctest::Context context;
	context.applyCommandLine(argc, argv);

	int res = context.run();
	return res;
}
#else
#include <iostream>
int main(void)
{
	std::cerr << "Compile with -DBIRB_TEST to build the tests\n";
	return 1;
}
#endif
