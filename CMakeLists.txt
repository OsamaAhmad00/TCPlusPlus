cmake_minimum_required(VERSION 3.30)
project(TCPlusPlus)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wpedantic -Wconversion -Wshadow")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -g3")
# address sanitizer and thread sanitizer are incompatible and can't be enabled together
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_executable(tcpp main.cpp)

include(CTest)
add_subdirectory(external)

if(BUILD_TESTING)
    add_executable(tests tests.cpp)
    target_link_libraries(tests PRIVATE GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(tests)
endif ()