#include <gtest/gtest.h>
#include <iostream>

namespace {

bool helloWorld() {
    std::cout << "Hello World!\n";
    return true;
}

}

TEST(tests, helloWorld) {
  EXPECT_TRUE(helloWorld());
}