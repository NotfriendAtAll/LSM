# tests/CMakeLists.txt
add_executable(unit_tests t_skip.cpp)
target_link_libraries(unit_tests 
  skiplist
  GTest::GTest 
  GTest::Main  # 自动生成main函数
  pthread       # Linux需显式链接
)

# 注册测试
add_test(NAME AllTests COMMAND unit_tests)