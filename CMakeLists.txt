cmake_minimum_required(VERSION 3.30)
project(TCPlusPlus)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wpedantic -Wconversion -Wshadow")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -g3")
# address sanitizer and thread sanitizer are incompatible and can't be enabled together
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

include_directories(include)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(SOURCE_FILES
    ${SOURCE_DIR}/TunDevice.cpp
    ${SOURCE_DIR}/structs/IPv4.cpp
    ${SOURCE_DIR}/utils/IPv4.cpp
    ${SOURCE_DIR}/utils/Checksum.cpp
)

add_executable(tcpp
    main.cpp
    ${SOURCE_FILES}
)

include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()