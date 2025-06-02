namespace Global_ {
constexpr int MAX_MEMTABLE_SIZE_PER_TABLE = 1024 * 1024 * 4;  // 4MB
enum class SkiplistStatus {
  kNormal,
  KFreezing,
  kFrozen,
};

}  // namespace Global_