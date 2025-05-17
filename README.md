# LSM 存储引擎项目
- 更新中
- 本项目为基于 C++ 的 LSM（Log-Structured Merge Tree）存储引擎实现，采用 GoogleTest 进行测试驱动开发。

## 目录结构

```
LSM/
├── include/           # 头文件目录
│   ├── Block.h       # 数据块相关定义
│   ├── BlockMeta.h   # 块元数据相关定义
│   ├── memtable.h    # 内存表相关定义
│   └── Skiplist.h    # 跳表相关定义
├── src/              # 源文件目录
├── test/             # 测试目录
│   ├── Block_test/   
│   ├── BlockMeat_test/
│   ├── Memtable_test/
│   └── Skiplist_test/
└── CMakeLists.txt
```

## 环境要求

- C++17 或更高版本
- CMake 3.10+
- GoogleTest

Ubuntu/Debian 环境下安装依赖：
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libgtest-dev
```

## 构建与测试

### 构建所有模块

```bash
mkdir -p build
cd build
cmake ..
make
```

### 运行测试

在 build 目录下执行：
```bash
# 运行所有测试
ctest

# 或运行单个模块测试
./test/Block_test/block_test
./test/BlockMeat_test/blockmeta_test
./test/Memtable_test/memtable_test
./test/Skiplist_test/skiplist_test
```

### 单独构建某个模块测试

以 BlockMeta 为例：
```bash
cd test/BlockMeat_test
mkdir -p build && cd build
cmake ..
make
./blockmeta_test
```

## 主要模块

### Skiplist
- 支持基本的增删改查操作
- 实现迭代器接口
- 支持事务ID

### MemTable
- 基于跳表实现的内存表
- 支持批量操作
- 实现并发控制

### Block
- 数据块的序列化与反序列化
- 支持二分查找
- 实现迭代器接口

### BlockMeta
- 块元数据的编码与解码
- 支持首尾键值对的管理
- 偏移量管理

## 测试覆盖

每个核心模块都配备了完整的单元测试：
- 常规操作测试
- 边界条件测试
- 并发操作测试（适用模块）
- 内存安全测试

测试使用 GoogleTest 框架，支持：
- 断言验证
- 测试夹具
- 参数化测试
- 测试输出格式化