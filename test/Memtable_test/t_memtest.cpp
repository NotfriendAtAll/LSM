#include "../../include/memtable.h"
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <vector>

class MemtableTest : public ::testing::Test {
protected:
    void SetUp() override {
        memtable = std::make_shared<MemTable>();
    }
    std::shared_ptr<MemTable> memtable;
};

// 基本的 put/get 操作测试
TEST_F(MemtableTest, BasicPutGet) {
    memtable->put("key1", "value1");
    auto result = memtable->get("key1");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "value1");
}

// 测试带锁的 put/get 操作
TEST_F(MemtableTest, MutexPutGet) {
    memtable->put_mutex("key1", "value1");
    std::vector<std::string> values;
    auto it = memtable->get_mutex("key1", values);
    values.emplace_back(it.getValue().second);
    EXPECT_FALSE(values.empty());
    EXPECT_EQ(values[0], "value1");
}


TEST_F(MemtableTest, BatchOperations) {
    std::vector<std::pair<std::string,std::string>> batch_data = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"}
    };
    
    // 测试批量插入
    memtable->put_batch(batch_data);
    
    // 测试批量获取
    std::vector<std::string> keys = {"key1", "key2", "key3"};
    auto result=memtable->get_batch(keys);
    EXPECT_EQ(result.size(), 3);
}

// 删除操作测试
TEST_F(MemtableTest, RemoveOperations) {
    // 普通删除
    memtable->put("key1", "value1");
    memtable->remove("key1",1);
    EXPECT_FALSE(memtable->get("key1").has_value());
    
    // 带锁的删除
    memtable->put("key2", "value2");
    memtable->remove_mutex("key2",1);
    EXPECT_FALSE(memtable->get("key2").has_value());
    
    // 批量删除
    std::vector<std::pair<std::string, std::string>> batch_data = {
        {"key3", "value3"},
        {"key4", "value4"}
    };  
    memtable->put_batch(batch_data);
    std::vector<std::string> keys_to_remove = {"key3", "key4"};
    memtable->remove_batch(keys_to_remove);
    EXPECT_FALSE(memtable->get("key3").has_value());
    EXPECT_FALSE(memtable->get("key4").has_value());
}

TEST_F(MemtableTest, TransactionIdTest) {
    memtable->put("key1", "value1", 1);
    memtable->put("key1", "value2", 2);
    
    std::vector<std::string> values;
    auto it = memtable->cur_get("key1", 1);
    values.push_back(it.getValue().second);
    EXPECT_FALSE(values.empty());
    EXPECT_EQ(values[0], "");
}

// 表冻结和刷新测试
TEST_F(MemtableTest, FrozenAndFlush) {
    memtable->put("key1", "value1");
    size_t initial_size = memtable->get_cur_size();
    
    // 测试冻结当前表
    memtable->frozen_cur_table();
    EXPECT_EQ(memtable->get_cur_size(), 0);
    EXPECT_GT(memtable->get_fixed_size(), 0);
    
    // 测试刷新操作
    memtable->flush();
    EXPECT_EQ(memtable->get_total_size(), memtable->get_fixed_size());
}

// 并发测试
TEST_F(MemtableTest, ConcurrentOperations) {
    std::vector<std::thread> threads;
    
    // 创建多个线程进行并发操作
    for(int i = 0; i < 10; i++) {
        threads.emplace_back([this, i]() {
            memtable->put_mutex("key" + std::to_string(i),                              "value" + std::to_string(i));
        });
    }
    
    // 等待所有线程完成
    for(auto& thread : threads) {
        thread.join();
    }
    
    // 验证所有数据都正确写入
    for(int i = 0; i < 10; i++) {
        auto result = memtable->get("key" + std::to_string(i));
        EXPECT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), "value" + std::to_string(i));
    }
}
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}