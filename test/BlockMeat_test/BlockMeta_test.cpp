#include "../../include/BlockMeta.h"
#include <gtest/gtest.h>
#include <string>
#include <vector>

class BlockMetaTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建一个测试用的 BlockMeta
        meta = BlockMeta("first_key", "last_key", 100);
    }
    BlockMeta meta;
};

// 测试构造函数
TEST_F(BlockMetaTest, Constructor) {
    BlockMeta meta1;  // 默认构造
    EXPECT_TRUE(meta1.first_key_.empty());
    EXPECT_TRUE(meta1.last_key_.empty());
    EXPECT_EQ(meta1.offset_, 0);

    BlockMeta meta2("key1", "key2", 123);  // 带参数构造
    EXPECT_EQ(meta2.first_key_, "key1");
    EXPECT_EQ(meta2.last_key_, "key2");
    EXPECT_EQ(meta2.offset_, 123);
}

// 测试编码和解码
TEST_F(BlockMetaTest, EncodeAndDecode) {
    std::vector<BlockMeta> metas;
    metas.push_back(BlockMeta("key1", "key2", 100));
    metas.push_back(BlockMeta("key3", "key4", 200));
    
    std::vector<uint8_t> encoded;
    meta.encode_meta_to_slice(metas, encoded);
    
    // 确保编码后的数据不为空
    ASSERT_FALSE(encoded.empty());
    
    // 解码并验证
    BlockMeta decoded_meta;
    decoded_meta.decode_meta_from_slice(encoded);
    
    // 验证第一个元数据
    EXPECT_EQ(decoded_meta.first_key_, "key1");
    EXPECT_EQ(decoded_meta.last_key_, "key2");
    EXPECT_EQ(decoded_meta.offset_, 100);
}

// 测试边界情况
TEST_F(BlockMetaTest, EdgeCases) {
    // 测试空字符串
    BlockMeta empty_meta("", "", 0);
    std::vector<BlockMeta> metas = {empty_meta};
    std::vector<uint8_t> encoded;
    
    empty_meta.encode_meta_to_slice(metas, encoded);
    EXPECT_FALSE(encoded.empty());
    
    BlockMeta decoded;
    decoded.decode_meta_from_slice(encoded);
    EXPECT_TRUE(decoded.first_key_.empty());
    EXPECT_TRUE(decoded.last_key_.empty());
    EXPECT_EQ(decoded.offset_, 0);
}

// 测试特殊字符
TEST_F(BlockMetaTest, SpecialCharacters) {
    BlockMeta special_meta("key!@#", "key$%^", 999);
    std::vector<BlockMeta> metas = {special_meta};
    std::vector<uint8_t> encoded;
    
    special_meta.encode_meta_to_slice(metas, encoded);
    
    BlockMeta decoded;
    decoded.decode_meta_from_slice(encoded);
    EXPECT_EQ(decoded.first_key_, "key!@#");
    EXPECT_EQ(decoded.last_key_, "key$%^");
    EXPECT_EQ(decoded.offset_, 999);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}