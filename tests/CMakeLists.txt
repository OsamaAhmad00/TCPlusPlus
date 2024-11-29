set(CMAKE_CXX_STANDARD 23)

add_subdirectory(googletest)

add_executable(tests
    HelloWorld.cpp
    Checksum.cpp
    ${SOURCE_FILES}
)
target_link_libraries(tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)
