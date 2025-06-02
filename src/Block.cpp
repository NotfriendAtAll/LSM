#include "../include/Block.h"
#include "../include/BlockIterator.h"
#include <cstddef>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

class BlockIterator;
Block::Block(std::size_t capacity) : capcity(capacity) {}
Block::Block() : capcity(4096) {
  // 默认构造函数，初始化容量为4096
}

std::vector<uint8_t> Block::encode() {
  // 计算总大小：数据段 + 偏移数组(每个偏移2字节) + 元素个数(2字节)
  size_t total_bytes =
      Data_.size() * sizeof(uint8_t) + Offset_.size() * sizeof(uint16_t) + sizeof(uint16_t);
  std::vector<uint8_t> encoded(total_bytes, 0);

  // 1. 复制数据段
  std::memcpy(encoded.data(), Data_.data(), Data_.size() * sizeof(uint8_t));

  // 2. 复制偏移数组
  size_t offset_pos = Data_.size() * sizeof(uint8_t);
  std::memcpy(encoded.data() + offset_pos,
              Offset_.data(),                    // vector 的连续内存起始位置
              Offset_.size() * sizeof(uint16_t)  // 总字节数
  );

  // 3. 写入元素个数
  size_t   num_pos      = Data_.size() * sizeof(uint8_t) + Offset_.size() * sizeof(uint16_t);
  uint16_t num_elements = Offset_.size();
  std::memcpy(encoded.data() + num_pos, &num_elements, sizeof(uint16_t));

  return encoded;
}
std::shared_ptr<Block> Block::decode(const std::vector<uint8_t>& encoded, bool with_hash) {
  // 使用 make_shared 创建对象
  auto block = std::make_shared<Block>();

  // 1. 安全性检查
  if (encoded.size() < sizeof(uint16_t)) {
    throw std::runtime_error("Encoded Data_ too small");
  }

  // 2. 读取元素个数
  uint16_t num_elements;
  size_t   num_elements_pos = encoded.size() - sizeof(uint16_t);
  if (with_hash) {
    num_elements_pos -= sizeof(uint32_t);
    auto     hash_pos = encoded.size() - sizeof(uint32_t);
    uint32_t hash_value;
    memcpy(&hash_value, encoded.data() + hash_pos, sizeof(uint32_t));

    uint32_t compute_hash = std::hash<std::string_view>{}(std::string_view(
        reinterpret_cast<const char*>(encoded.data()), encoded.size() - sizeof(uint32_t)));
    if (hash_value != compute_hash) {
      throw std::runtime_error("Block hash verification failed");
    }
  }
  memcpy(&num_elements, encoded.data() + num_elements_pos, sizeof(uint16_t));

  // 3. 验证数据大小
  size_t required_size = sizeof(uint16_t) + num_elements * sizeof(uint16_t);
  if (encoded.size() < required_size) {
    throw std::runtime_error("Invalid encoded Data_ size");
  }

  // 4. 计算各段位置
  size_t offsets_section_start = num_elements_pos - num_elements * sizeof(uint16_t);

  // 5. 读取偏移数组
  block->Offset_.resize(num_elements);
  memcpy(block->Offset_.data(), encoded.data() + offsets_section_start,
         num_elements * sizeof(uint16_t));

  // 6. 复制数据段
  block->Data_.reserve(offsets_section_start);  // 优化内存分配
  block->Data_.assign(encoded.begin(), encoded.begin() + offsets_section_start);

  return block;
}
std::string Block::get_first_key() {
  if (Offset_.empty()) {
    return "";
  }
  return get_key(Offset_[0]);
}

std::optional<size_t> Block::get_idx_binary(const std::string& key, uint64_t tranc_id) {
  if (Offset_.empty()) {
    return std::nullopt;
  }
  // 二分查找
  size_t left  = 0;
  size_t right = Offset_.size() - 1;
  while (left <= right) {
    size_t      mid     = left + (right - left) / 2;
    std::string mid_key = get_key(Offset_[mid]);
    if (mid_key == key) {
      return mid;
    } else if (mid_key < key) {
      left = mid + 1;
    } else {
      right = mid - 1;
    }
  }
  return std::nullopt;
}
std::size_t Block::get_offset(const std::size_t index) {
  if (index >= Offset_.size()) {
    throw std::out_of_range("Index out of range");
  }
  return Offset_[index];
}
size_t Block::get_cur_size() const {
  return Data_.size() + Offset_.size() * sizeof(uint16_t) + sizeof(uint16_t);
}
std::optional<uint64_t> Block::get_tranc_id(const std::size_t offset) const {
  uint16_t key_len;
  memcpy(&key_len, Data_.data() + offset, sizeof(uint16_t));
  uint16_t value_len;
  memcpy(&value_len, Data_.data() + offset + sizeof(uint16_t) + key_len, sizeof(uint16_t));
  uint64_t tranc_id;
  memcpy(&tranc_id,
         Data_.data() + offset + sizeof(uint16_t) + key_len + sizeof(uint16_t) + value_len,
         sizeof(uint64_t));
  return tranc_id;
}
std::optional<std::string> Block::get_value_binary(const std::string& key) {
  auto idx = get_idx_binary(key);
  if (!idx) {
    return std::nullopt;
  }
  return get_value(Offset_[*idx]);
}
std::pair<std::string, std::string> Block::get_first_and_last_key() {
  if (Offset_.empty()) {
    return {"", ""};
  }
  std::string first_key = get_key(Offset_[0]);
  std::string last_key  = get_key(Offset_[Offset_.size() - 1]);
  return {first_key, last_key};
}
bool Block::add_entry(const std::string& key, const std::string& value, uint64_t tranc_id) {
  if ((get_cur_size() + key.size() + value.size() + 3 * sizeof(uint16_t) > capcity) &&
      !Offset_.empty()) {
    return false;
  }
  // 计算entry大小：key长度(2B) + key + value长度(2B) + value
  size_t entry_size =
      sizeof(uint16_t) + key.size() + sizeof(uint16_t) + value.size() + sizeof(uint64_t);
  size_t old_size = Data_.size();
  Data_.resize(old_size + entry_size);

  // 写入key长度
  uint16_t key_len = key.size();
  memcpy(Data_.data() + old_size, &key_len, sizeof(uint16_t));

  // 写入key
  memcpy(Data_.data() + old_size + sizeof(uint16_t), key.data(), key_len);

  // 写入value长度
  uint16_t value_len = value.size();
  memcpy(Data_.data() + old_size + sizeof(uint16_t) + key_len, &value_len, sizeof(uint16_t));

  // 写入value
  memcpy(Data_.data() + old_size + sizeof(uint16_t) + key_len + sizeof(uint16_t), value.data(),
         value_len);
  memcpy(Data_.data() + old_size + sizeof(uint16_t) + key_len + sizeof(uint16_t) + value_len,
         &tranc_id, sizeof(uint64_t));
  // 记录偏移
  Offset_.push_back(old_size);
  return true;
}
bool Block::is_empty() const {
  return Data_.empty() && Offset_.empty();
}
BlockIterator Block::begin() {
  auto shared = shared_from_this();
  return BlockIterator(shared, 0, 0);
}
BlockIterator Block::end() {
  auto shared = shared_from_this();
  return BlockIterator(shared, Offset_.size(), 0);
}

std::optional<std::pair<std::shared_ptr<BlockIterator>, std::shared_ptr<BlockIterator>>>
Block::get_prefix_iterator(std::string key, uint64_t tranc_id) {
  auto result1 = get_idx_binary(key, tranc_id);
  if (!result1.has_value()) {
    return std::nullopt;
  }
  auto result2 = get_idx_binary(key + '\xff', tranc_id);
  if (!result2.has_value()) {
    return std::make_pair(
        std::make_shared<BlockIterator>(shared_from_this(), result1.value(), tranc_id),
        std::make_shared<BlockIterator>(shared_from_this(), Offset_.size(), tranc_id));
  }
  auto begin = std::make_shared<BlockIterator>(shared_from_this(), result1.value(), tranc_id);
  auto end   = std::make_shared<BlockIterator>(shared_from_this(), result2.value(), tranc_id);
  return std::make_pair(begin, end);
}
std::string Block::get_key(const std::size_t offset) const {
  uint16_t key_len;
  memcpy(&key_len, Data_.data() + offset, sizeof(uint16_t));
  return std::string(reinterpret_cast<const char*>(Data_.data() + offset + sizeof(uint16_t)),
                     key_len);
}
std::string Block::get_value(const std::size_t offset) const {
  uint16_t key_len;
  memcpy(&key_len, Data_.data() + offset, sizeof(uint16_t));
  uint16_t value_len;
  memcpy(&value_len, Data_.data() + offset + sizeof(uint16_t) + key_len, sizeof(uint16_t));
  std::string value(value_len, '\0');
  memcpy(value.data(), Data_.data() + offset + sizeof(uint16_t) + key_len + sizeof(uint16_t),
         value_len);
  return value;
}
std::shared_ptr<Block::Entry> Block::get_entry(std::size_t offset, size_t index) {
  if (index > Offset_.size() || offset < 0 || Offset_.empty()) {
    throw std::out_of_range("Index out of range");
  }
  auto entry   = std::make_shared<Entry>();
  entry->key   = get_key(offset);
  entry->value = get_value(offset);
  return entry;
}