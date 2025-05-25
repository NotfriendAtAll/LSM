#include "../../include/Skiplist.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>


class SkiplistTest : public ::testing::Test {
protected:
    void SetUp() override {
        skiplist = std::make_unique<Skiplist>();
    }
    
    std::unique_ptr<Skiplist> skiplist;
};

// 基本插入和查询测试
TEST_F(SkiplistTest, BasicInsertAndGet) {
    EXPECT_TRUE(skiplist->Insert("key1", "value1"));
    EXPECT_TRUE(skiplist->Insert("key2", "value2"));
    
    auto result1 = skiplist->Contain("key1");
    EXPECT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "value1");
    
    auto result2 = skiplist->Contain("key2");
    EXPECT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "value2");
}

// 测试不存在的键
TEST_F(SkiplistTest, NotExistKey) {
    auto result = skiplist->Contain("notexist");
    EXPECT_FALSE(result.has_value());
}

// 测试删除功能
TEST_F(SkiplistTest, DeleteTest) {
    EXPECT_TRUE(skiplist->Insert("key1", "value1"));
    EXPECT_TRUE(skiplist->Delete("key1"));
    auto result = skiplist->Contain("key1");
    EXPECT_FALSE(result.has_value());
}

// 测试迭代器
TEST_F(SkiplistTest, IteratorTest) {
    std::vector<std::pair<std::string, std::string>> inputs = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"}
    };
    
    for (const auto& pair : inputs) {
        EXPECT_TRUE(skiplist->Insert(pair.first, pair.second));
    }
    
    auto it = skiplist->begin();
    int count = 0;
    while (it.valid()) {
        auto value = it.getValue();
        EXPECT_EQ(value.first, inputs[count].first);
        EXPECT_EQ(value.second, inputs[count].second);
        ++it;
        ++count;
    }
    EXPECT_EQ(count, inputs.size());
}

// 测试事务ID
TEST_F(SkiplistTest, TransactionIdTest) {
    EXPECT_TRUE(skiplist->Insert("key1", "value1", 1));
    EXPECT_TRUE(skiplist->Insert("key1", "value2", 2));
    
    auto result1 = skiplist->Contain("key1", 1);
    EXPECT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "value2");
    
    auto result2 = skiplist->Contain("key1", 2);
    EXPECT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "value2");
}

// 测试内存大小统计
TEST_F(SkiplistTest, SizeTest) {
    size_t initial_size = skiplist->get_size();
    EXPECT_TRUE(skiplist->Insert("key1", "value1"));
    EXPECT_GT(skiplist->get_size(), initial_size);
}

// 性能测试
TEST_F(SkiplistTest, PerformanceTest) {
    const int NUM_OPERATIONS = 1000000;  // 测试10万次操作
    std::vector<std::string> keys;
    keys.reserve(NUM_OPERATIONS);

    // 准备测试数据
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        keys.push_back("key" + std::to_string(i));
    }

    // 测试插入性能
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& key : keys) {
        skiplist->Insert(key, "value" + key);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto insert_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "插入 " << NUM_OPERATIONS << " 个元素耗时: " 
              << insert_duration.count() << " ms" << std::endl;

    // 测试查找性能
    start = std::chrono::high_resolution_clock::now();
    for (const auto& key : keys) {
        auto result = skiplist->Contain(key);
        EXPECT_TRUE(result.has_value());
    }
    end = std::chrono::high_resolution_clock::now();
    auto lookup_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "查找 " << NUM_OPERATIONS << " 个元素耗时: " 
              << lookup_duration.count() << " ms" << std::endl;

    // 测试删除性能
    start = std::chrono::high_resolution_clock::now();
    for (const auto& key : keys) {
        EXPECT_TRUE(skiplist->Delete(key));
    }
    end = std::chrono::high_resolution_clock::now();
    auto delete_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "删除 " << NUM_OPERATIONS << " 个元素耗时: " 
              << delete_duration.count() << " ms" << std::endl;
}

// 内存使用测试
TEST_F(SkiplistTest, MemoryUsageTest) {
    const std::vector<int> TEST_SIZES = {1000, 10000, 100000}; 
    std::vector<double> avg_bytes_per_node;
    
    for (int size : TEST_SIZES) {
        skiplist = std::make_unique<Skiplist>();
        size_t initial_size = skiplist->get_size();
        
        for (int i = 0; i < size; i++) {
            std::string key = "key" + std::to_string(i);
            std::string value = "value" + std::to_string(i);
            EXPECT_TRUE(skiplist->Insert(key, value));
        }
        
        size_t memory_used = skiplist->get_size() - initial_size;
        double bytes_per_node = (double)memory_used / size;
        avg_bytes_per_node.push_back(bytes_per_node);
        
        std::cout << "数据量: " << size << " 条记录" << std::endl;
        std::cout << "总内存占用: " << memory_used << " bytes" << std::endl;
        std::cout << "平均每节点内存: " << bytes_per_node << " bytes" << std::endl;
        
        // 验证每个节点的内存使用在合理范围内
        // 基础结构（key+value+指针数组）至少需要12字节
        EXPECT_GT(bytes_per_node, 12.0);
        // 考虑平均层数，不应该超过基础大小的2倍
        EXPECT_LT(bytes_per_node, 24.0);
        
        // 清理数据
        skiplist.reset();
    }
    
    // 验证内存增长趋势的合理性
    for (size_t i = 1; i < avg_bytes_per_node.size(); i++) {
        double growth_ratio = avg_bytes_per_node[i] / avg_bytes_per_node[i-1];
        std::cout << "节点平均内存增长比例: " << growth_ratio << std::endl;
        
        // 每增加10倍数据量，平均节点大小增长不应超过15%
        EXPECT_LT(growth_ratio, 1.15);
    }
}

// 范围查询测试
TEST_F(SkiplistTest, RangeSearchTest) {
    // 插入测试数据
    std::vector<std::pair<std::string, std::string>> test_data = {
        {"key01", "value1"},
        {"key02", "value2"},
        {"key03", "value3"},
        {"key04", "value4"},
        {"key05", "value5"},
        {"other1", "other1"},
        {"prefix1", "prefix1"}
    };
    
    for (const auto& pair : test_data) {
        EXPECT_TRUE(skiplist->Insert(pair.first, pair.second));
    }

    // 测试前缀范围查询
    auto begin_it = skiplist->prefix_serach_begin("key");
    auto end_it = skiplist->prefix_serach_end("key");
    EXPECT_TRUE(begin_it.getValue().first == "key01");
    std::cout << "begin_it: " << begin_it.getValue().first << std::endl;
    std::cout << "end_it: " << end_it.getValue().first << std::endl;

    // 验证范围内的元素
    std::vector<std::pair<std::string, std::string>> range_results;
    for (auto it = begin_it; it != end_it; ++it) {
        range_results.push_back(it.getValue());
    }   

    // 验证结果数量
    EXPECT_EQ(range_results.size(), 5);  // 应该只有5个key0x的元素

    // 验证结果顺序和内容
    for (size_t i = 0; i < range_results.size(); i++) {
        EXPECT_EQ(range_results[i].first, "key0" + std::to_string(i + 1));
        EXPECT_EQ(range_results[i].second, "value" + std::to_string(i + 1));
    }

    // 测试空范围
    auto empty_begin = skiplist->prefix_serach_begin("nonexist");
    auto empty_end = skiplist->prefix_serach_end("nonexist");
    EXPECT_EQ(empty_begin, empty_end);

    // 测试事务ID的范围查询
    EXPECT_TRUE(skiplist->Insert("key06", "value6_v1", 1));
    EXPECT_TRUE(skiplist->Insert("key06", "value6_v2", 2));

    auto tx_begin = skiplist->prefix_serach_begin("key06", 2);
    auto tx_result = tx_begin.getValue();
    EXPECT_EQ(tx_result.second, "value6_v2");  // 应该看到最新版本

    // 测试边界情况
    auto begin_empty = skiplist->prefix_serach_begin("");
    auto end_empty = skiplist->prefix_serach_end("");
    EXPECT_TRUE(begin_empty.valid());  // 空前缀应该返回第一个元素
    
    // 测试迭代器移动
    auto it = skiplist->prefix_serach_begin("key");
    EXPECT_TRUE(it.valid());
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(it.valid());
        ++it;
    }
        
}



