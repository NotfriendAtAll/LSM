#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>

class Block;
class BlockIterator;
bool operator==(const BlockIterator& lhs, const BlockIterator& rhs) noexcept;

class BlockIterator {
 public:
  friend bool operator==(const BlockIterator& lhs, const BlockIterator& rhs) noexcept;
  // 标准迭代器类型定义
  using iterator_category = std::forward_iterator_tag;
  using value_type        = std::pair<std::string, std::string>;
  using difference_type   = std::ptrdiff_t;
  using pointer           = value_type*;
  using reference         = value_type&;
  using con_pointer       = const value_type*;
  using con_reference     = const value_type&;

  // 构造函数
  BlockIterator();
  BlockIterator(std::shared_ptr<Block> block_, const std::string& key, uint64_t tranc_id = 0);
  BlockIterator(std::shared_ptr<Block> block_, size_t index, uint64_t tranc_id = 0);

  bool is_end();

  // 迭代器操作
  con_pointer    operator->();
  BlockIterator& operator++();
  BlockIterator  operator++(int) = delete;
  value_type     operator*();

  auto       operator<=>(const BlockIterator& rhs) const -> std::strong_ordering;
  value_type getValue() const;
  size_t     getIndex() const;

 private:
  void update_current();
  // 跳过当前不可见事务的id (如果开启了事务功能)
  void skip_by_tranc_id();

 private:
  std::shared_ptr<Block>    block;          // 指向所属的 Block
  size_t                    current_index;  // 当前位置的索引
  uint64_t                  tranc_id_;      // 当前事务 id
  std::optional<value_type> cached_value;   // 缓存当前值
};