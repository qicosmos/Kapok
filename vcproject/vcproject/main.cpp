#include <array>
#include <vector>
#include <string>
#include <iostream>
#include "kapok/Kapok.hpp"

void test()
{
	Serializer sr;
	DeSerializer dr;

	auto tp = std::make_tuple(10, 12, string("test"));
	sr.Serialize(tp, "tuple");
	dr.Parse(sr.GetString());
	std::tuple<int, int, string> p;
	dr.Deserialize(p, "tuple");
}

template<class T>
void test_array(const T& arr)
{
	Serializer sr;
	sr.Serialize(arr, "test");
	std::cout << sr.GetString() << std::endl;

	DeSerializer dr;
	dr.Parse(sr.GetString());
	T de_arr;
	dr.Deserialize(de_arr, "test");
}

int main()
{
	std::array<std::string, 5> arr = { "a","b","c", "d", "e" };
	test_array(arr);

	std::vector<std::string> vt = { "a","b","c", "d", "e" };
	test_array(vt);

	test();

	return 0;
}