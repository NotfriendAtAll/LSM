#include "../include/BlockIterator.h"
#include "../include/Block.h"
#include <optional>
#include <sys/types.h>

BlockIterator::BlockIterator()
    : block(nullptr), current_index(0), tranc_id_(0) {}
BlockIterator::BlockIterator(std::shared_ptr<Block> block_,
                           const std::string &key, size_t index,uint64_t tranc_id)
    : block(block_),current_index(index),tranc_id_(tranc_id) {  // 初始化为0
    if (!block) {
        return;
    }
    
    if (!key.empty()) {
        auto iter = block->get_idx_binary(key);
        if (iter.has_value()) {
            current_index = iter.value();
        } else {
            current_index = block->Offset_.size();
        }
    } else if (index < block->Offset_.size()) {
        current_index = index;
    }
    
    if (current_index < block->Offset_.size()) {
        update_current();
    }
}

bool BlockIterator::is_end() {
  if (block) {
    return current_index >= block->Offset_.size();
  }
  return true;
}
BlockIterator::con_pointer BlockIterator::operator->() const {
  if (cached_value.has_value()) {
    return &(*cached_value);
  }
update_current();
  return &(*cached_value);
}

bool BlockIterator::operator!=(const BlockIterator &other) const {
  if (block&&other.block) {
    return !(block==other.block&&current_index==other.current_index);
}
return true;
}
bool BlockIterator::operator==(const BlockIterator &other) const {
  if (block&&other.block) {
    return block==other.block&&current_index==other.current_index;
  }
  return false;
}

BlockIterator& BlockIterator::operator++() {
    if (block && current_index < block->Offset_.size()) {
        ++current_index;
        update_current();
    }
    return *this;
}
void BlockIterator::update_current() const {
    cached_value = std::nullopt;  // 每次都清空缓存
    if (block && current_index < block->Offset_.size()) {
        auto offset = block->Offset_[current_index];
        auto entry = block->get_entry(offset);
        cached_value = std::make_pair(entry.key, entry.value);
    }
}
void BlockIterator::skip_by_tranc_id() {
  if (tranc_id_ == 0) {
    cached_value = std::nullopt;
    return;
  }
  while (current_index < block->Offset_.size()) {
    auto offset = block->Offset_[current_index];
    auto entry = block->get_entry(offset);
    if (entry.tranc_id != tranc_id_) {
      break;
    }
    ++current_index;
  }
}
BlockIterator::value_type BlockIterator::operator*() const {
    if (!cached_value.has_value()) {
        update_current();
    }
    if (!cached_value.has_value()) {
        throw std::runtime_error("Dereferencing invalid iterator");
    }
    return *cached_value;
}
BlockIterator::value_type BlockIterator::getValue() const {
  auto offset = block->Offset_[current_index];
  auto entry = block->get_entry(offset);
  return {entry.key, entry.value};
}
  size_t BlockIterator::getIndex() const {
  return current_index;
}