SET(TEST_SOURCES
	test.cpp
    primitive.cpp
    stl.cpp
    user.cpp
)

INCLUDE_DIRECTORIES(..)
ADD_EXECUTABLE(kapok_test ${TEST_SOURCES})
TARGET_LINK_LIBRARIES(kapok_test lua)
ADD_CUSTOM_TARGET(run_test COMMAND ${CMAKE_BINARY_DIR}/test/kapok_test DEPENDS kapok_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
