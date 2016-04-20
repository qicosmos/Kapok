#pragma once
#define TEST_MAIN
#include "unit_test.hpp"

int test_func(int a, int b)
{
	return a + b;
}

TEST_CASE(test_add)
{
	TEST_CHECK(test_func(1,2) == 3);
}

