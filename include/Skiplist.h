#pragma once
#include "BaseIterator.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <utility>
#include <vector>

const int MAX_LEVEL = 16;
class skiplist;
class Node {
public:
  std::string key_;
  std::string value_;
  std::vector<std::shared_ptr<Node>> forward;
  uint64_t transaction_id;
  Node(const std::string &key, const std::string &value, int level,uint64_t transaction_ids = 0)
      : key_(key), value_(value), forward(level, nullptr), transaction_id(transaction_ids) {}
  bool operator>(const Node &other) const;
  bool operator<(const Node &other) const;
};

class SkiplistIterator : public BaseIterator {
  friend class Skiplist; // 让 Skiplist 可以访问私有成员
public:
  using valuetype = std::pair<std::string, std::string>;
  SkiplistIterator(std::shared_ptr<Node> skiplist);
  SkiplistIterator();
  virtual BaseIterator &operator++() override;
  ~SkiplistIterator() = default;
   bool valid() const override;
   bool operator==(const BaseIterator &other) const override;
   bool operator!=(const BaseIterator &other) const override;
   IteratorType type() const override;
   bool isEnd() override;
   bool operator*() const override;
 uint64_t getseq() const override;
 std::pair<std::string,std::string> getValue() const;

private:
  std::shared_ptr<Node> current;
};

class Skiplist {
public:
  Skiplist(int max_level_ = MAX_LEVEL)
      : max_level(max_level_), current_level(1), size_bytes(0),
        nodecount(0), gen(rd()), dis(0.0, 1.0) {
    head = std::make_shared<Node>("", "", max_level_);
  } // 默认最大16层

  bool Insert(const std::string &key, const std::string &value,
              uint64_t transaction_id = 0);
  std::optional<std::string> Contain(const std::string &key,
                                     uint64_t transaction_id = 0);
  bool Delete(const std::string &key);
  std::shared_ptr<Node> Get(const std::string &key,uint64_t transaction_id = 0);
  SkiplistIterator end();
  SkiplistIterator begin();
  std::vector<std::pair<std::string, std::string>> flush();
  std::size_t get_size();
  auto seekToFirst();
  auto seekToLast();
  std::size_t getnodecount();

private:
  std::shared_ptr<Node> head;
  int max_level;     // 最大层级
  int current_level; // 当前层级
  size_t size_bytes; // 内存占用，达到。flush到disk
  int random_level();
  std::size_t nodecount = 0; // 节点数量
  std::random_device rd; // 随机数生成器
  std::mt19937 gen; // 随机数引擎
  std::uniform_real_distribution<> dis; // 随机数分布
};
