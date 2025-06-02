#include "../include/std_file.h"

// 打开文件，可选择创建新文件
bool StdFile::open(const std::string& filename, bool create) {
  filename_                    = filename;
  std::ios_base::openmode mode = std::ios::in | std::ios::out | std::ios::binary;
  if (create) {
    mode |= std::ios::trunc;
  }
  file_.open(filename, mode);
  return file_.is_open();
}

// 创建新文件并写入缓冲区内容
bool StdFile::create(const std::string& filename, std::vector<uint8_t>& buf) {
  if (!open(filename, true)) {
    throw std::runtime_error("Failed to open file for writing");
  }
  if (!buf.empty()) {
    if (!write(0, buf.data(), buf.size())) {
      throw std::runtime_error("Failed to write buffer to file");
    }
  }
  return true;
}

// 关闭文件
void StdFile::close() {
  if (file_.is_open()) {
    sync();
    file_.close();
  }
}

// 获取文件大小
size_t StdFile::size() {
  if (!file_.is_open()) {
    throw std::runtime_error("File not open");
  }
  auto current_pos = file_.tellg();
  file_.seekg(0, std::ios::end);
  size_t file_size = file_.tellg();
  file_.seekg(current_pos, std::ios::beg);  // 恢复原位置
  return file_size;
}

// 从指定偏移读取指定长度内容
std::vector<uint8_t> StdFile::read(size_t offset, size_t length) {
  if (!file_.is_open()) {
    throw std::runtime_error("File not open");
  }
  std::vector<uint8_t> buf(length);
  file_.seekg(0, std::ios::end);
  size_t file_size = file_.tellg();
  if (offset > file_size) {
    throw std::out_of_range("Read offset is out of file size");
  }
  size_t read_len = std::min(length, file_size - offset);
  file_.seekg(offset, std::ios::beg);
  if (!file_.read(reinterpret_cast<char*>(buf.data()), read_len)) {
    throw std::runtime_error("Failed to read from file");
  }
  buf.resize(read_len);  // 实际读取长度
  return buf;
}

// 从指定偏移写入数据
bool StdFile::write(size_t offset, const void* data, size_t size) {
  if (!file_.is_open()) {
    return false;
  }
  file_.seekp(offset, std::ios::beg);
  file_.write(static_cast<const char*>(data), size);
  return file_.good();
}

// 同步数据到磁盘
bool StdFile::sync() {
  if (!file_.is_open()) {
    return false;
  }
  file_.flush();
  return file_.good();
}

// 删除文件
bool StdFile::remove() {
  close();
  return std::remove(filename_.c_str()) == 0;
}
