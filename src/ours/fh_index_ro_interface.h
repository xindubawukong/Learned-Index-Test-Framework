#include "../competitor/indexInterface.h"
#include "./fh_index_ro/src/include/fh_index_ro/fh_index_ro.h"

const size_t kBlockSize = 64;
const size_t kFingerDensity = 8;
const size_t kInternalBlockSize = 16;
const size_t kInternalFingerDensity = 2;

const size_t kTopLevelThreshold = 1024;

template <class KEY_TYPE, class PAYLOAD_TYPE>
class FHIndexROInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
public:
  static_assert(std::is_same_v<KEY_TYPE, uint64_t>,
                "FH_index_RO must have uint64_t key type");

  void init(Param *param = nullptr) {}

  void bulk_load(std::pair<KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,
                 Param *param = nullptr) {
    std::vector<std::pair<KEY_TYPE, PAYLOAD_TYPE>> entries;
    entries.assign(key_value, key_value + num);
    index.BulkLoad(entries);
  }

  bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr) {
    size_t lb = index.LowerBound(key);
    val = index.leaf_level_.GetValue(lb);
    return true;
  }

  bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {
    assert(0);
  }

  bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {
    assert(0);
  }

  bool remove(KEY_TYPE key, Param *param = nullptr) { assert(0); }

  size_t scan(KEY_TYPE key_low_bound, size_t key_num,
              std::pair<KEY_TYPE, PAYLOAD_TYPE> *result,
              Param *param = nullptr) {
    assert(0);
  }

  long long memory_consumption() { return index.IndexSize(); }

private:
  fh_index_ro::FGIndexRO<uint64_t, kTopLevelThreshold, kBlockSize,
                           kFingerDensity, kInternalBlockSize, kInternalFingerDensity>
        index;
};