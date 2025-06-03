#include "../include/BaseIterator.h"

auto operator<=>(const SerachIterator& lhs, const SerachIterator& rhs) -> std::strong_ordering {
  if (lhs.key_ != rhs.key_) {
    return lhs.key_ <=> rhs.key_;
  }
  if (lhs.transaction_id_ != rhs.transaction_id_) {
    return lhs.transaction_id_ <=> rhs.transaction_id_;
  }
  if (lhs.level_ != rhs.level_) {
    return lhs.level_ <=> rhs.level_;
  }
  return lhs.index_ <=> rhs.index_;
}

/*bool operator<(const SerachIterator& lhs, const SerachIterator& rhs) {
  if (lhs.key_ != rhs.key_) {
    return lhs.key_ < rhs.key_;
  }
  if (lhs.transaction_id_ > rhs.transaction_id_) {
    return true;
  }
  if (lhs.level_ < rhs.level_) {
    return true;
  }
  return lhs.index_ < rhs.index_;
}
bool operator>(const SerachIterator& lhs, const SerachIterator& rhs) {
  if (lhs.key_ != rhs.key_) {
    return lhs.key_ > rhs.key_;
  }
  if (lhs.transaction_id_ < rhs.transaction_id_) {
    return true;
  }
  if (lhs.level_ > rhs.level_) {
    return true;
  }
  return lhs.index_ > rhs.index_;
}
bool operator==(const SerachIterator& lhs, const SerachIterator& rhs) {
  return lhs.key_ == rhs.key_ && lhs.value_ == rhs.value_ &&
         lhs.transaction_id_ == rhs.transaction_id_;
}*/