cmake_minimum_required(VERSION 3.30)
project(TCPlusPlus)

set(CMAKE_CXX_STANDARD 23)

add_executable(TCPlusPlus main.cpp)

include(CTest)
add_subdirectory(external)

if(BUILD_TESTING)
    add_executable(tests tests.cpp)
    target_link_libraries(tests PRIVATE GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(tests)
endif ()