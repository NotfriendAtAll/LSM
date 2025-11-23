cmake_minimum_required(VERSION 4.0)
project(LSM_Sstable_Test)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

# 查找 GTest 包
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# 添加源文件
set(SOURCE_FILES
    ../../src/Sstable.cpp
    ../../src/SstableIterator.cpp
    ../../src/Block.cpp
    ../../src/BlockIterator.cpp
    ../../src/Blockcache.cpp
    ../../src/std_file.cpp
    ../../src/mmap.cpp
    ../../src/file.cpp
    ../../src/BloomFilter.cpp
    ../../src/BlockMeta.cpp
    ../../src/memtable.cpp
    ../../src/Skiplist.cpp
    ../../src/Baselterator.cpp
    ../../src/Global.cpp
)

# 添加测试文件
add_executable(sstable_test
    sstable_test.cpp
    ${SOURCE_FILES}
)

# 链接 Google Test 和线程库
target_link_libraries(sstable_test
    ${GTEST_LIBRARIES}
    pthread
)

# 添加头文件路径
target_include_directories(sstable_test PRIVATE
    ../../include
)

# 启用测试
enable_testing()
add_test(NAME sstable_test COMMAND sstable_test)