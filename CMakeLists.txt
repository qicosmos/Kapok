cmake_minimum_required(VERSION 2.8)

project(kapok)
add_definitions(FMT_HEADER_ONLY)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DEBUG")
    #add_definitions(
    #    -D_DEBUG
    #)
endif ()

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

SET(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

include_directories(
	${PROJECT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/fmt
)

set(SOURCE_FILES 
	main.cpp
		test/panic.cpp
		test/primitive.cpp
		test/stl.cpp
		test/user.cpp
	)


add_executable(kapok ${SOURCE_FILES})
target_link_libraries(kapok ${EXTRA_LIBS})

#install(FILES src/cloud_backup.ini DESTINATION "${PROJECT_BINARY_DIR}/")

####################################
# test

# enable_testing()
# add_test(fundation_test_run fundation_test)
# set_tests_properties (fundation_test_run PROPERTIES PASS_REGULAR_EXPRESSION "0 failures are detected")

# add_test(get_object_summary_test_run get_object_summary_test)
# set_tests_properties (get_object_summary_test_run PROPERTIES PASS_REGULAR_EXPRESSION "0 failures are detected")

# add_test(list_object_summary_test_run list_object_summary_test)
# set_tests_properties (list_object_summary_test_run PROPERTIES PASS_REGULAR_EXPRESSION "0 failures are detected")

# add_test(test_backup_status_run test_backup_status)
# set_tests_properties (test_backup_status_run PROPERTIES PASS_REGULAR_EXPRESSION "0 failures are detected")

# add_test(test_kv_store_run test_kv_store)
# set_tests_properties (test_kv_store_run PROPERTIES PASS_REGULAR_EXPRESSION "0 failures are detected")

# add_test(test_kv_traverse_delete_run test_kv_traverse_delete)
# set_tests_properties (test_kv_traverse_delete_run PROPERTIES PASS_REGULAR_EXPRESSION "0 failures are detected")

# add_test(test_thread_pool_run test_thread_pool)

# add_test(test_scribe_load_balance_run test_scribe_load_balance)


include (InstallRequiredSystemLibraries)
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "David Doria")
include (CPack)