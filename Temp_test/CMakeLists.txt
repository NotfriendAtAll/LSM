cmake_minimum_required(VERSION 4.0)
project(MyProject)

# 设置C++标准
set(CMAKE_CXX_STANDARD 23)

# 添加被测代码
add_library(skiplist STATIC ../src/Skiplist.cpp)
target_include_directories(skiplist PUBLIC ../include)

# 引入GoogleTest（根据实际路径调整）
# add_subdirectory(../googletest/build googletest)  # 若源码在项目内
# 或使用系统安装版本：
find_package(GTest REQUIRED)
add_subdirectory(../test/Skiplist_test  ${CMAKE_BINARY_DIR}/Skiplist_test_build )

enable_testing()
 