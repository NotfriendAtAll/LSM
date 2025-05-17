#include <cstdint>
#include <string>
#include <utility>
enum class IteratorType {
  SkiplistIterator,
  MemTableIterator,
  BlockIterator,
  SSTableIterator,
};

class BaseIterator {
public:
  using valuetype = std::pair<std::string, std::string>;
  using pvaluetype=valuetype*;
  using rvaluetype=valuetype&;
  virtual ~BaseIterator() = default;
  virtual bool valid() const = 0;
  virtual BaseIterator &operator++() = 0;
  virtual bool operator==(const BaseIterator &other) const = 0;
  virtual bool operator!=(const BaseIterator &other) const = 0;
  virtual IteratorType type() const = 0;
  virtual bool isEnd() = 0;
  virtual bool operator*() const = 0;
  virtual uint64_t getseq() const = 0;
};
class SerachIterator {
public:
  std::string key_;
  std::string value_;
  uint64_t transaction_id_;
  size_t level_;
  size_t index_;
  SerachIterator(std::string key, std::string value, uint64_t transaction_id,
    size_t level,size_t index)
      : key_(std::move(key)), value_(std::move(value)),
      transaction_id_(transaction_id) ,level_(level),index_(index) {}
  SerachIterator() = default;
  ~SerachIterator() = default;
};
bool operator <(const SerachIterator &lhs, const SerachIterator &rhs);
bool operator>(const SerachIterator &lhs, const SerachIterator &rhs);
bool operator==(const SerachIterator &lhs, const SerachIterator &rhs);
