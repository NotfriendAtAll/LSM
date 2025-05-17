#include "../include/memtable.h"
#include "../include/Global.h"
#include <cstddef>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <utility>

MemTable::MemTable() {
  current_table = std::make_shared<Skiplist>(MAX_LEVEL);
  fixed_bytes = 0;
}
MemTable::~MemTable()=default;
void MemTable::put(const std::string &key, const std::string &value,
                   uint64_t transaction_id) {
  current_table->Insert(key, value, transaction_id);
}

void MemTable::put_mutex(const std::string &key, const std::string &value,
                          uint64_t transaction_id) {
  std::unique_lock<std::shared_mutex> lock(cur_lock_);
  current_table->Insert(key, value, transaction_id);
  if (current_table->get_size() > Global_::MAX_MEMTABLE_SIZE_PER_TABLE) {
    std::unique_lock<std::shared_mutex> lock(fix_lock_);
    frozen_cur_table();
  }
}
void MemTable::put_batch(
    const std::vector<std::pair<std::string, std::string>> &key_value_pairs,
    uint64_t transaction_id) {
  for (const auto &pair : key_value_pairs) {
    current_table->Insert(pair.first, pair.second, transaction_id);
  }
  if (current_table->get_size() > Global_::MAX_MEMTABLE_SIZE_PER_TABLE) {
    std::unique_lock<std::shared_mutex> lock(fix_lock_);
    frozen_cur_table();
  }
}
SkiplistIterator MemTable::cur_get(const std::string &key,             
                                     uint64_t transaction_id) {
  std::shared_lock<std::shared_mutex> lock(cur_lock_);
  return SkiplistIterator(current_table->Get(key,transaction_id));
}
SkiplistIterator MemTable::fix_get(const std::string &key,                            
                                     uint64_t transaction_id) {
  std::shared_lock<std::shared_mutex> lock(fix_lock_);
  for (const auto &result : fixed_tables) {
    if (result->Contain(key,transaction_id).has_value()) {
      return SkiplistIterator(result->Get(key,transaction_id));
    }
  }
  return SkiplistIterator();
}
SkiplistIterator MemTable::get_mutex(
    const std::string &key, std::vector<std::string> &values) {
  std::shared_lock<std::shared_mutex> lock(cur_lock_);
  return SkiplistIterator(current_table->Get(key));
  lock.unlock();
  std::shared_lock<std::shared_mutex> second_lock(fix_lock_);
  for (const auto &result : fixed_tables) {
    if (result->Contain(key).has_value()) {
      return SkiplistIterator(result->Get(key));
    }
  }
  return SkiplistIterator();
}
 
std::size_t MemTable::get_fixed_size() {
  std::shared_lock<std::shared_mutex> lock(fix_lock_);
  return fixed_bytes;
}
std::size_t MemTable::get_cur_size() {
  std::shared_lock<std::shared_mutex> lock(cur_lock_);
  return current_table->get_size();
}
std::optional<std::string> MemTable::get(const std::string &key) {
  std::shared_lock<std::shared_mutex> lock(cur_lock_);
  auto result = current_table->Contain(key);
  if (result.has_value()) {
    // 检查是否为删除标记
    if (result.value().empty()) {
      return std::nullopt;  // 如果是空值，表示该key已被删除
    }
    return result;
  }
  
  lock.unlock();
  std::shared_lock<std::shared_mutex> second_lock(fix_lock_);
  for (const auto &fixed_table : fixed_tables) {
    auto result = fixed_table->Contain(key);
    if (result.has_value()) {
      // 同样检查固定表中的删除标记
      if (result.value().empty()) {
        return std::nullopt;
      }
      return result;
    }
  }
  return std::nullopt;
}
std::vector<std::tuple<std::string, std::optional<std::string>,
                        std::optional<uint64_t>>>
MemTable::get_batch(const std::vector<std::string> &key_pairs, uint64_t transaction_id) {
  std::vector<std::tuple<std::string, std::optional<std::string>, std::optional<uint64_t>>> result;
  for (const auto &pair : key_pairs) {
    auto value = get(pair);
    if (value.has_value() && !value.value().empty()) {  // 添加空值检查
      result.emplace_back(pair, value, transaction_id);
    } else {
      result.emplace_back(pair, std::nullopt, std::nullopt);
    }
  }
  return result;
}
std::size_t MemTable::get_total_size() {
  return get_cur_size() + get_fixed_size();
}

void MemTable::frozen_cur_table() {
  std::unique_lock<std::shared_mutex> lock(cur_lock_);
  auto new_table = std::make_shared<Skiplist>(MAX_LEVEL);
  auto temp_size = current_table->get_size();
  current_table->flush();
  std::unique_lock<std::shared_mutex> lock2(fix_lock_);
  fixed_tables.push_back(current_table);
  current_table = new_table;
  fixed_bytes += temp_size;
  }

void MemTable::remove(const std::string &key, uint64_t transaction_id) {
  current_table->Insert(key, "", transaction_id);
  if (current_table->get_size() > Global_::MAX_MEMTABLE_SIZE_PER_TABLE) {
    std::unique_lock<std::shared_mutex> lock(fix_lock_);
    frozen_cur_table();
  }
}
void MemTable::remove_mutex(const std::string &key, uint64_t transaction_id) {
  std::unique_lock<std::shared_mutex> lock(cur_lock_);
  current_table->Insert(key, "", transaction_id);
  if (fixed_tables.size() > Global_::MAX_MEMTABLE_SIZE_PER_TABLE) {
    std::unique_lock<std::shared_mutex> lock(fix_lock_);
    frozen_cur_table();
  }
}
// 批量删除
void MemTable::remove_batch(
    const std::vector<std::string> &key_pairs,uint64_t transaction_id) {
  std::unique_lock<std::shared_mutex> lock(cur_lock_);
  for (const auto &pair : key_pairs) {
    current_table->Insert(pair, "", transaction_id);
  }
  if (current_table->get_size() > Global_::MAX_MEMTABLE_SIZE_PER_TABLE) {
    std::unique_lock<std::shared_mutex> lock(fix_lock_);
    frozen_cur_table();
  }
}
// 批量刷新磁盘
void MemTable::flush_batch(
    const std::vector<std::pair<std::string, std::string>> &key_value_pairs) {
  std::unique_lock<std::shared_mutex> lock(cur_lock_);
  for (const auto &pair : key_value_pairs) {

    current_table->Insert(pair.first, pair.second);
  }
  auto new_table = std::make_shared<Skiplist>(MAX_LEVEL);
  current_table->flush();
  fixed_tables.push_back(current_table);
  current_table = new_table;
  fixed_bytes += current_table->get_size();
}
// This function is used to flush the current memtable to disk,刷新到磁盘
void MemTable::flush() {
  std::unique_lock<std::shared_mutex> lock(cur_lock_);
  auto new_table = std::make_shared<Skiplist>(MAX_LEVEL);
  current_table->flush();
  fixed_tables.push_back(current_table);
  current_table = new_table;
  fixed_bytes += current_table->get_size();
  if (fixed_tables.size() > Global_::MAX_MEMTABLE_SIZE_PER_TABLE) {
    fixed_tables.pop_front();
  }
}
MemTableIterator MemTable::begin() {
  return MemTableIterator(fixed_tables.begin()->get()->begin(),0);
}
MemTableIterator MemTable::end() {
  return MemTableIterator(fixed_tables.end()->get()->end(),0);
}
// 迭代器
MemTableIterator::MemTableIterator(const SkiplistIterator &iter, uint64_t transaction_id)
    : max_transaction_id(transaction_id) {
  list_iter_ = std::make_shared<SkiplistIterator>(iter);
}
MemTableIterator::MemTableIterator(std::vector<SerachIterator> iter,uint64_t transaction_id)
    : max_transaction_id(0) {
  for (auto  it : iter) {
    queue_.push(std::move(it));
  }
  while (!top_value_legal()) {
skip_transaction_id();
while (!queue_.empty()&&queue_.top().value_.empty()) {
auto temp = queue_.top().key_;
while (!queue_.empty()&&queue_.top().key_ == temp) {
  queue_.pop();
}
}  
  }
}
void MemTableIterator::skip_transaction_id() {
  if (max_transaction_id==0) {
  return;
  }
  while (!queue_.empty() && queue_.top().transaction_id_ <= max_transaction_id) {
    queue_.pop();
  }
}
bool MemTableIterator::top_value_legal()const{
  if (max_transaction_id==0) {
    return !queue_.top().value_.empty();
  }
  if (queue_.empty()) {
    return true;  
  }
  if (queue_.top().transaction_id_<max_transaction_id) {
    return !queue_.top().value_.empty(); 
  }
  else {
    return false;
  }
}
bool MemTableIterator::valid() const {
  return !queue_.empty();
}
bool MemTableIterator::operator==(const BaseIterator &other) const {
  if (other.type() != IteratorType::MemTableIterator) {
    return false;
  }
  const MemTableIterator &other_iter =
      dynamic_cast<const MemTableIterator &>(other);
      if (other_iter.queue_.empty()|| queue_.empty()) {
      return other_iter.queue_.empty()&&queue_.empty()? true : false;
      }
  return queue_.top().key_ == other_iter.queue_.top().key_ &&
         queue_.top().value_ == other_iter.queue_.top().value_ &&
         queue_.top().transaction_id_ == other_iter.queue_.top().transaction_id_;
}
bool MemTableIterator::operator!=(const BaseIterator &other) const {
  return !(*this == other);
}
bool MemTableIterator::isEnd() {
  return queue_.empty();
}
bool MemTableIterator::operator*() const {
  return !queue_.empty();
}
uint64_t MemTableIterator::getseq() const {
  if (queue_.empty()) {
    return 0;
  }
  return queue_.top().transaction_id_;
}
MemTableIterator &MemTableIterator::operator++() {
  if (queue_.empty()) {
    return *this;
  }
  auto temp = queue_.top().key_;
  while (!queue_.empty()&&queue_.top().key_ == temp) {
    queue_.pop();
  }
  while (!top_value_legal()) {
    skip_transaction_id();
    while (!queue_.empty()&&queue_.top().value_.empty()) {
      auto temp = queue_.top().key_;
      while (!queue_.empty()&&queue_.top().key_ == temp) {
        queue_.pop();
      }
    }
  }
  return *this;
}
IteratorType MemTableIterator::type() const {
  return IteratorType::MemTableIterator;
}
void MemTableIterator::update_current_key_value()const {
  if (!queue_.empty()) {
  current_value_ = std::make_shared<valuetype>(queue_.top().key_, queue_.top().value_);
}
  else {
    current_value_.reset();
  }
}
BaseIterator::pvaluetype MemTableIterator::operator->() const {
update_current_key_value();
  return current_value_.get();
}
