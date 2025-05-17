cmake_minimum_required(VERSION 3.20)
project(LSM_Memtable_Test)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 GTest 包
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 添加源文件
set(SOURCE_FILES
    ../../src/memtable.cpp
    ../../src/Skiplist.cpp
    ../../src/Baselterator.cpp
)

# 添加测试文件
add_executable(memtable_test
    t_memtest.cpp
    ${SOURCE_FILES}
)

# 链接 Google Test 和线程库
target_link_libraries(memtable_test
    ${GTEST_LIBRARIES}
    pthread
)

# 添加头文件路径
target_include_directories(memtable_test PRIVATE
    ../../include
)

# 启用测试
enable_testing()
add_test(NAME memtable_test COMMAND memtable_test)