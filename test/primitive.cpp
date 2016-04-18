#include "UnitTest.hpp"
#include "kapok/Kapok.hpp"

TEST_CASE(char_serialize)
{
    Serializer sr;
    sr.Serialize('c', "char value");
    TEST_CHECK(sr.GetString() == std::string(R"({"char value":99})"));
}

TEST_CASE(char_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"char value":99})");
    char v = 0;
    dr.Deserialize(v, "char value");
    TEST_CHECK(v == 'c');
}

TEST_CASE(int_serialize)
{
    Serializer sr;
    sr.Serialize(0, "int value");
    TEST_CHECK(sr.GetString() == std::string(R"({"int value":0})"));
}

TEST_CASE(int_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"int value":0})");
    int v = -1;
    dr.Deserialize(v, "int value");
    TEST_CHECK(v == 0);
}

TEST_CASE(array_serialize)
{
    Serializer sr;
    int a[] = {1, 2, 3, 4, 5};
    sr.Serialize(a, "array");
    TEST_CHECK(sr.GetString() == std::string(R"({"array":[1,2,3,4,5]})"));
}

TEST_CASE(array_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"array":[1,2,3,4,5]})");
    int v[5];
    dr.Deserialize(v, "array");
    TEST_CHECK(v[0] == 1);
    TEST_CHECK(v[1] == 2);
    TEST_CHECK(v[2] == 3);
    TEST_CHECK(v[3] == 4);
    TEST_CHECK(v[4] == 5);
}
