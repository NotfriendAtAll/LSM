cmake_minimum_required(VERSION 3.30)
cmake_policy(SET CMP0167 NEW)  # 启用NEW行为，直接使用BoostConfig.cmake
project(LSM_Block_Test)
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# 查找 GTest 包
find_package(GTest REQUIRED)


# 添加源文件
set(SOURCE_FILES
    ../../src/Block.cpp
    ../../src/BlockIterator.cpp
)

# 添加测试文件
add_executable(block_test
    Blocktest.cpp
    ${SOURCE_FILES}
)

# 链接库 - 使用正确的目标名
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