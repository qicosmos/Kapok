#include "UnitTest.hpp"
#include "kapok/Kapok.hpp"
#include <map>

TEST_CASE(user_type_with_primitive)
{
    using namespace kapok;
    struct T
    {
        int a;
        char b;
        
        META(a, b);
    };

    Serializer sr;
    sr.Serialize(T{66, 'c'}, "T");
    TEST_CHECK(sr.GetString() == std::string(R"({"T":{"a":66,"b":99}})"));
    
    T t;
    DeSerializer dr;
    dr.Parse(R"({"T":{"a":66,"b":99}})");
    dr.Deserialize(t, "T");
    
    TEST_CHECK(t.a == 66);
    TEST_CHECK(t.b == 'c');
}

TEST_CASE(user_type_with_string)
{
	using namespace kapok;
    struct T
    {
        std::string a;
        std::string b;

        META(a, b);
    };

    Serializer sr;
    sr.Serialize(T{"str1", "str2"}, "T");
    TEST_CHECK(sr.GetString() == std::string(R"({"T":{"a":"str1","b":"str2"}})"));

    DeSerializer dr;
    dr.Parse(R"({"T":{"a":"str1","b":"str2"}})");
    T t;
    dr.Deserialize(t, "T");
    TEST_CHECK(t.a == "str1");    
    TEST_CHECK(t.b == "str2");    
}

TEST_CASE(user_type_with_container)
{
	using namespace kapok;
    struct T
    {
        std::map<int, std::string> a;
        std::set<int> b;
        std::vector<std::string> c;  

        META(a, b, c);
    };

    Serializer sr;
    sr.Serialize(T {
        std::map<int, std::string>{{1, "1"}, {2, "2"}, {3, "3"}},
        std::set<int>{1, 2, 3, 4, 5},
        std::vector<std::string>{"1", "2", "3", "4", "5"}
    }, "T");

    TEST_CHECK(sr.GetString() == std::string(R"({"T":{"a":{"1":"1","2":"2","3":"3"},"b":[1,2,3,4,5],"c":["1","2","3","4","5"]}})"));
    DeSerializer dr;
    dr.Parse(R"({"T":{"a":{"1":"1","2":"2","3":"3"},"b":[1,2,3,4,5],"c":["1","2","3","4","5"]}})");
    T t;
    dr.Deserialize(t, "T");
    TEST_CHECK(t.a == (std::map<int, std::string>{{1, "1"}, {2, "2"}, {3, "3"}}));
    TEST_CHECK(t.b == (std::set<int>{1, 2, 3, 4, 5}));
    TEST_CHECK(t.c == (std::vector<std::string>{"1", "2", "3", "4", "5"}));
}
