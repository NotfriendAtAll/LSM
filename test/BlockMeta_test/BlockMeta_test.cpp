#include "../../include/BlockMeta.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

class BlockMetaTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试用的 BlockMeta 对象
        meta = BlockMeta("first", "last", 1024);
    }
    BlockMeta meta;
};

// 测试基本构造函数
TEST_F(BlockMetaTest, BasicConstructors) {
    // 测试默认构造
    BlockMeta meta1;
    EXPECT_TRUE(meta1.first_key_.empty());
    EXPECT_TRUE(meta1.last_key_.empty());
    EXPECT_EQ(meta1.offset_, 0);

    // 测试带参数构造
    BlockMeta meta2("first", "last", 100);
    EXPECT_EQ(meta2.first_key_, "first");
    EXPECT_EQ(meta2.last_key_, "last");
    EXPECT_EQ(meta2.offset_, 100);
}

// 测试编码和解码功能
TEST_F(BlockMetaTest, EncodeDecode) {
    std::vector<BlockMeta> test_metas = {
        BlockMeta("key1", "key2", 100),
        BlockMeta("key3", "key4", 200),
        BlockMeta("key5", "key6", 300)
    };
    
    std::vector<uint8_t> encoded;
    meta.encode_meta_to_slice(test_metas, encoded);
    
    // 验证编码后的数据不为空
    ASSERT_FALSE(encoded.empty());
    
    // 测试解码
    auto decoded = meta.decode_meta_from_slice(encoded);
    ASSERT_EQ(decoded.size(), test_metas.size());
    
    // 验证解码后的数据正确性
    for (size_t i = 0; i < test_metas.size(); i++) {
        EXPECT_EQ(decoded[i].first_key_, test_metas[i].first_key_);
        EXPECT_EQ(decoded[i].last_key_, test_metas[i].last_key_);
        EXPECT_EQ(decoded[i].offset_, test_metas[i].offset_);
    }
}

// 测试边界情况
TEST_F(BlockMetaTest, EdgeCases) {
    // 测试空列表编码解码
    std::vector<BlockMeta> empty_metas;
    std::vector<uint8_t> encoded;
    
    meta.encode_meta_to_slice(empty_metas, encoded);
    auto decoded = meta.decode_meta_from_slice(encoded);
    EXPECT_TRUE(decoded.empty());
    
    // 测试包含空字符串的元数据
    std::vector<BlockMeta> test_metas = {
        BlockMeta("", "", 0),
        BlockMeta("key", "", 100),
        BlockMeta("", "last", 200)
    };
    
    meta.encode_meta_to_slice(test_metas, encoded);
    decoded = meta.decode_meta_from_slice(encoded);
    
    ASSERT_EQ(decoded.size(), test_metas.size());
    EXPECT_TRUE(decoded[0].first_key_.empty());
    EXPECT_TRUE(decoded[0].last_key_.empty());
    EXPECT_EQ(decoded[0].offset_, 0);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}