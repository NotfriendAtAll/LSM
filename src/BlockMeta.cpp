#include "../include/BlockMeta.h"
#include <cstring>


BlockMeta::BlockMeta() : first_key_(""), last_key_(""), offset_(0) {}
BlockMeta::BlockMeta(std::string first_key, std::string last_key, size_t offset)
    : first_key_(first_key), last_key_(last_key), offset_(offset) {}
void BlockMeta::encode_meta_to_slice(std::vector<BlockMeta>& meta, std::vector<uint8_t>& slice) {
    size_t size = meta.size();
    slice.resize(size * sizeof(BlockMeta));
    for (size_t i = 0; i < size; ++i) {
        auto& block_meta = meta[i];
        memcpy(slice.data() + i * sizeof(BlockMeta), &block_meta, sizeof(BlockMeta));
    }
}
void BlockMeta::decode_meta_from_slice(std::vector<uint8_t>& slice) {
    size_t size = slice.size() / sizeof(BlockMeta);
    std::vector<BlockMeta> meta(size);
    for (size_t i = 0; i < size; ++i) {
        memcpy(&meta[i], slice.data() + i * sizeof(BlockMeta), sizeof(BlockMeta));
    }
}