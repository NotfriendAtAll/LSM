cmake_minimum_required(VERSION 3.10)
project(SkiplistTest)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# 查找 GTest 包
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 添加源文件
add_executable(skiplist_test
    skiplist_test.cpp
    ../../src/Skiplist.cpp
)

# 链接 Google Test 库
target_link_libraries(skiplist_test
    GTest::GTest
    GTest::Main
    pthread
)

# 添加头文件路径
target_include_directories(skiplist_test PRIVATE
    ../../include
)

# 启用测试
enable_testing()
add_test(NAME skiplist_test COMMAND skiplist_test)