#include "../include/BlockIterator.h"
#include "../include/Block.h"
#include <boost/stacktrace.hpp>
#include <boost/stacktrace/stacktrace.hpp>
#include <optional>
#include <utility>

bool operator==(const BlockIterator& lhs, const BlockIterator& rhs) noexcept {
  return lhs.block == rhs.block && lhs.current_index == rhs.current_index &&
         lhs.tranc_id_ == rhs.tranc_id_;
}
BlockIterator::BlockIterator() : block(nullptr), current_index(0), tranc_id_(0) {}
BlockIterator::BlockIterator(std::shared_ptr<Block> block_, const std::string& key,
                             uint64_t tranc_id)
    : block(block_), tranc_id_(tranc_id) {  // 初始化为0
  if (!block) {
    return;
  }
  auto iter = block->get_idx_binary(key, tranc_id);
  if (iter.has_value()) {
    current_index = iter.value();
  } else {
    current_index = block->Offset_.size();
  }
}
BlockIterator::BlockIterator(std::shared_ptr<Block> block_, size_t index, uint64_t tranc_id)
    : block(block_), current_index(index), tranc_id_(tranc_id) {
  skip_by_tranc_id();  // 跳过不可见的事务
}

bool BlockIterator::is_end() {
  if (block) {
    return current_index >= block->Offset_.size();
  }
  return true;
}
BlockIterator::con_pointer BlockIterator::operator->() {
  if (cached_value.has_value()) {
    return &(*cached_value);
  }
  update_current();
  return &(*cached_value);
}
BlockIterator& BlockIterator::operator++() {
  if (block && current_index < block->Offset_.size()) {
    current_index++;
    update_current();
    skip_by_tranc_id();  // 跳过不可见的事务
  }
  return *this;
}
BlockIterator::value_type BlockIterator::operator*() {
  if (!cached_value.has_value()) {
    update_current();
  }
  if (!cached_value.has_value()) {
    throw std::runtime_error("Dereferencing invalid iterator");
  }
  return *cached_value;
}
auto BlockIterator::operator<=>(const BlockIterator& other) const -> std::strong_ordering {
  if (block != other.block) {
    return block <=> other.block;
  }
  if (current_index != other.current_index) {
    return current_index <=> other.current_index;
  }
  return tranc_id_ <=> other.tranc_id_;
}

BlockIterator::value_type BlockIterator::getValue() const {
  if (current_index < 0 || current_index > block->Offset_.size()) {
    throw std::out_of_range("Index out of range in BlockIterator");
  }
  auto offset = block->Offset_[current_index];
  auto entry  = block->get_entry(offset, current_index);
  return std::make_pair(entry->key, entry->value);
}
size_t BlockIterator::getIndex() const {
  return current_index;
}
void BlockIterator::update_current() {
  cached_value = std::nullopt;  // 每次都清空缓存
  if (block && current_index < block->Offset_.size()) {
    auto offset  = block->Offset_[current_index];
    auto entry   = block->get_entry(offset, current_index);
    cached_value = std::make_pair(entry->key, entry->value);
  }
}
void BlockIterator::skip_by_tranc_id() {
  if (tranc_id_ == 0) {
    cached_value = std::nullopt;
    return;
  }
  while (current_index < block->Offset_.size()) {
    auto offset = block->Offset_[current_index];
    auto entry  = block->get_entry(offset, current_index);
    if (entry->tranc_id < tranc_id_) {
      break;
    }
    ++current_index;
  }
}