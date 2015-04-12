#include "kapok/Kapok.hpp"

void Performance()
{
	Serializer sr;
	DeSerializer dr;
	
	auto tp = std::make_tuple(10, 12, string("test"));
	Timer t;
	for (size_t i = 0; i < 10000; i++)
	{
		sr.Serialize(tp, "tuple");
		dr.Parse(sr.GetString());
		std::tuple<int, int, string> p;
		dr.Deserialize(p, "tuple");
	}

	int64_t elapsed = t.elapsed();
	cout << "kapok: " << elapsed <<"ms"<< endl;
}
