cmake_minimum_required(VERSION 3.20)
project(LSM_Block_Test)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 GTest 包
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 添加源文件
set(SOURCE_FILES
    ../../src/Block.cpp
    ../../src/Blocklterator.cpp
)

# 添加测试文件
add_executable(block_test
    Blocktest.cpp
    ${SOURCE_FILES}
)

# 链接 Google Test 库
target_link_libraries(block_test
    GTest::GTest
    GTest::Main
    pthread
)

# 添加头文件路径
target_include_directories(block_test PRIVATE
    ../../include
)

# 启用测试
enable_testing()
add_test(NAME block_test COMMAND block_test)