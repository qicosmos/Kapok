#include "UnitTest.hpp"
#include "kapok/Kapok.hpp"

TEST_CASE(user_type_with_primitive)     //FIXME: not compile
{
    struct T
    {
        int a;
        char b;
        
        META(a, b);
    };

    Serializer sr;
    T t{66, 'c'};
    sr.Serialize(t, "T");
    std::cout << sr.GetString() << std::endl;
}
