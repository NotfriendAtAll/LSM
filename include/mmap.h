#pragma once
#include <cstdint>
#include <string>
#include <vector>

class MmapFile {
 public:
  MmapFile() : fd_(-1), mapped_data_(nullptr), file_size_(0) {}
  ~MmapFile() { close(); }

  // 打开文件并映射到内存
  bool open(const std::string& filename, bool create = false);

  // 创建文件
  bool create(const std::string& filename, const std::vector<uint8_t>& buf);

  // 写入数据
  bool write(size_t offset, const void* data, size_t size);

  // 读取数据
  std::vector<uint8_t> read(size_t offset, size_t length);

  // 同步到磁盘
  bool sync();

  // 关闭文件
  void close();

  // 获取文件大小
  size_t size() const { return file_size_; }

 private:
  int         fd_;           // 文件描述符
  void*       mapped_data_;  // 映射的内存地址
  size_t      file_size_;    // 文件大小
  std::string filename_;     // 文件名

  // 获取映射的内存指针
  void* data() const { return mapped_data_; }

  // 创建文件并映射到内存
  bool create_and_map(const std::string& path, size_t size);

  // 禁止拷贝
  MmapFile(const MmapFile&)            = delete;
  MmapFile& operator=(const MmapFile&) = delete;
};

/*封装了**mmap（内存映射文件）**的操作。
允许将磁盘文件直接映射到内存空间，实现高性能的数据访问（尤其适合大文件或频繁随机读写的场景）。
提供了打开、创建、读写、同步（sync）、关闭等接口。
用于那些对性能敏感、需要直接内存操作的场景，比如大数据块的顺序访问或随机访问。
*/