#include <msgpack.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <boost/timer.hpp>
#include <kapok/Kapok.hpp>
#include <fmt/format.h>

const int MAXSIZE = 1000000;

struct person
{
	std::string name;
	int age;

	MSGPACK_DEFINE_MAP(name, age);
};

void test_msgpack()
{
	person p = { "test", 20 };

	msgpack::sbuffer sbuf;

	boost::timer tm;
	for (size_t i = 0; i < MAXSIZE; i++)
	{
		sbuf.clear();
		msgpack::pack(sbuf, p);
	}
	std::cout << tm.elapsed() << " ";

	person rp;
	tm.restart();

	for (size_t i = 0; i < MAXSIZE; i++)
	{
		msgpack::unpack(sbuf.data(), sbuf.size()).get().convert(rp);
	}
	std::cout << tm.elapsed() << std::endl;
}

struct my_person
{
	std::string name;
	int age;

	META(name, age);
};

void test_kapok()
{
	my_person p = { "test", 20 };

    kapok::Serializer sr;
	boost::timer tm;
	for (size_t i = 0; i < MAXSIZE; i++)
	{
		sr.Serialize(p);
	}
	std::cout << tm.elapsed() << " ";

	tm.restart();
	my_person rp;
	kapok::DeSerializer dr;
	for (size_t i = 0; i < MAXSIZE; i++)
	{
		dr.Parse(sr.GetString());
		dr.Deserialize(rp);
	}
	std::cout << tm.elapsed() << std::endl;
}

void test_fmt()
{
	boost::timer tm;
	fmt::MemoryWriter wr;
	for (size_t i = 0; i < MAXSIZE; i++)
	{
		wr << 20;
		wr.c_str();
		wr.clear();
	}
	std::cout << tm.elapsed() << std::endl;
}

void test_boost_cast()
{
	boost::timer tm;
	for (size_t i = 0; i < MAXSIZE; i++)
		boost::lexical_cast<std::string>(20);

	std::cout << tm.elapsed() << std::endl;
}

void test_kapok_all()
{
	my_person p = { "test", 20 };
	my_person rp;

	kapok::Serializer sr;
	kapok::DeSerializer dr;
	boost::timer tm;
	for (size_t i = 0; i < MAXSIZE; i++)
	{
		sr.Serialize(p);
		dr.Parse(sr.GetString());
		dr.Deserialize(rp);
	}
	std::cout << tm.elapsed() << " kapok" << std::endl;
}

void test_msgpack_all()
{
	person p = { "test", 20 };
	person rp;

	msgpack::sbuffer sbuf;

	boost::timer tm;
	for (size_t i = 0; i < MAXSIZE; i++)
	{
		sbuf.clear();
		msgpack::pack(sbuf, p);
		msgpack::unpack(sbuf.data(), sbuf.size()).get().convert(rp);
	}
	std::cout << tm.elapsed() << " msgpack" << std::endl;
}

int main(void) {
	//test_fmt();
	//test_boost_cast();

	test_msgpack();
	test_kapok();

	//test_msgpack_all();
	//test_kapok_all();
}
