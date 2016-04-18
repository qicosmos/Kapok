#include "UnitTest.hpp"
#include "kapok/Kapok.hpp"
#include <map>
#include <unordered_map>

TEST_CASE(string_serialize)
{
    Serializer sr;
    sr.Serialize("hello, world", "string value");
    TEST_CHECK(sr.GetString() == std::string(R"({"string value":"hello, world"})"));  
}

TEST_CASE(string_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"string value":"hello, world"})");
    std::string v;
    dr.Deserialize(v, "string value");
    TEST_CHECK(v == "hello, world");
}

TEST_CASE(array_serialize)
{
    Serializer sr;
    sr.Serialize((std::array<int, 5>{1, 2, 3, 4, 5}), "array");   
    TEST_CHECK(sr.GetString() == std::string(R"({"array":[1,2,3,4,5]})"));
}

TEST_CASE(array_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"array":[1,2,3,4,5]})");
    std::array<int, 5> v;
    std::array<int, 5> a{1, 2, 3, 4, 5};
    dr.Deserialize(v, "array");                                      
    TEST_CHECK(v == a);
}

TEST_CASE(vector_serialize)
{
    Serializer sr;
    sr.Serialize(std::vector<int>{1, 2, 3, 4, 5}, "vector");
    TEST_CHECK(sr.GetString() == std::string(R"({"vector":[1,2,3,4,5]})"));
}

TEST_CASE(vector_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"vector":[1,2,3,4,5]})");
    std::vector<int> v;
    dr.Deserialize(v, "vector");
    TEST_CHECK(v == (std::vector<int>{1, 2, 3, 4, 5}));
}

TEST_CASE(queue_serialize)
{
    Serializer sr;
    std::queue<int> q; q.push(1); q.push(2); q.push(3); q.push(4); q.push(5);
    sr.Serialize(q, "queue");
    TEST_CHECK(sr.GetString() == std::string(R"({"queue":[1,2,3,4,5]})"));
}

TEST_CASE(queue_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"queue":[1,2,3,4,5]})");
    std::queue<int> v;
    dr.Deserialize(v, "queue");
    for(int i = 0; i < 5; ++i)
    {
        TEST_CHECK(v.front() == i + 1);
        v.pop();
    }
}

TEST_CASE(stack_serialize)
{
    Serializer sr;
    std::stack<int> q; q.push(5); q.push(4); q.push(3); q.push(2); q.push(1);
    sr.Serialize(q, "stack");
    TEST_CHECK(sr.GetString() == std::string(R"({"stack":[1,2,3,4,5]})"));
}

TEST_CASE(stack_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"stack":[1,2,3,4,5]})");
    std::stack<int> v;
    dr.Deserialize(v, "stack");
    for(int i = 0; i < 5; ++i)
    {
        TEST_CHECK(v.top() == i + 1); 
        v.pop();
    }
}

TEST_CASE(set_serialize)
{
    Serializer sr;
    sr.Serialize(std::set<int>{1, 2, 3, 4, 5}, "set");
    TEST_CHECK(sr.GetString() == std::string(R"({"set":[1,2,3,4,5]})"));
}

TEST_CASE(set_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"set":[1,2,3,4,5]})");
    std::set<int> v;
    dr.Deserialize(v, "set");
    TEST_CHECK(v == (std::set<int>{1, 2, 3, 4, 5}));
}

TEST_CASE(unordered_set_serialize_and_deserialize)
{
    Serializer sr;
    sr.Serialize(std::unordered_set<int>{1, 2, 3, 4, 5}, "set");
        
    std::unordered_set<int> v;
    DeSerializer dr;
    dr.Parse(sr.GetString());
    dr.Deserialize(v, "set");
    TEST_CHECK(v == (std::unordered_set<int>{1, 2, 3, 4, 5}));
}

TEST_CASE(unordered_set_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"set":[1,2,3,4,5]})");
    std::unordered_set<int> v;
    dr.Deserialize(v, "set");
    TEST_CHECK(v == (std::unordered_set<int>{1, 2, 3, 4, 5}));
}

TEST_CASE(map_serialize)
{
    Serializer sr;
    sr.Serialize(std::map<std::string, int>{{"1", 1}, {"2", 2}, {"3", 3}}, "map");
    TEST_CHECK(sr.GetString() == std::string(R"({"map":{"1":1,"2":2,"3":3}})"));
}

TEST_CASE(map_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"map":{"1":1,"2":2,"3":3}})");
    std::map<std::string, int> v;
    dr.Deserialize(v, "map");
    TEST_CHECK(v == (std::map<std::string, int>{{"1", 1}, {"2", 2}, {"3", 3}}));
}

TEST_CASE(unordered_map_serialize_and_deserialize)
{
    Serializer sr;
    sr.Serialize(std::unordered_map<std::string, int>{{"1", 1}, {"2", 2}, {"3", 3}}, "map");

    std::unordered_map<std::string, int> v;
    DeSerializer dr;
    dr.Parse(sr.GetString());
    dr.Deserialize(v, "map");
    TEST_CHECK(v == (std::unordered_map<std::string, int>{{"1", 1}, {"2", 2}, {"3", 3}}));
}

TEST_CASE(unordered_map_deserialize)
{
    DeSerializer dr;
    dr.Parse(R"({"map":{"1":1,"2":2,"3":3}})");
    std::unordered_map<std::string, int> v;
    dr.Deserialize(v, "map");
    TEST_CHECK(v == (std::unordered_map<std::string, int>{{"1", 1}, {"2", 2}, {"3", 3}}));
}

TEST_CASE(stl_with_user_type)
{
    struct T
    {
        std::string key;
        std::string val;

        bool operator==(const T& t) const
        {
            return key == t.key;
        }

        META(key, val);
    };

    Serializer sr;
    sr.Serialize(std::vector<T>{T{"k1", "v1"}, T{"k2", "v2"}, T{"k3", "v3"}});
    
    DeSerializer dr;
    dr.Parse(R"({"temp":[{"key":"k1","val":"v1"},{"key":"k2","val":"v2"},{"key":"k3","val":"v3"}]})");
    std::vector<T> v;
    dr.Deserialize(v);
    TEST_CHECK(v == (std::vector<T>{{"k1", "v1"}, T{"k2", "v2"}, T{"k3", "v3"}})); 
}
