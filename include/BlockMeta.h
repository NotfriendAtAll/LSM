#include <string>
#include <vector>
#include <cstdint>

class BlockMeta {
 public:
  BlockMeta();
  BlockMeta(std::string first_key, std::string last_key, size_t offset);
  void encode_meta_to_slice(std::vector<BlockMeta>& meta, std::vector<uint8_t>& slice);
  std::vector<BlockMeta> decode_meta_from_slice(std::vector<uint8_t>& slice);

  std::string first_key_;
  std::string last_key_;
  size_t      offset_;
};