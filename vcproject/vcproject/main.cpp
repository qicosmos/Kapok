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

int main()
{
	test();

	return 0;
}