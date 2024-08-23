#include "../competitor/indexInterface.h"
#include "./fh_index_utils.h"
#include "fh_index_ro/fh_index_ro.h"

template <class KEY_TYPE, class PAYLOAD_TYPE>
class FHIndexROInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
 public:
  static_assert(std::is_same_v<KEY_TYPE, uint64_t>,
                "FH_index_RO must have uint64_t key type");

  void init(Param *param = nullptr) {}

  void bulk_load(std::pair<KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,
                 Param *param = nullptr) {
    index.BulkLoad(
        num, [&](size_t i) { return key_value[i].first; },
        [&](size_t i) { return key_value[i].second; });
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
    size_t lb = index.LowerBound(key_low_bound);
    size_t cnt = 0;
    while (lb < index.leaf_level_.keys_.size() && cnt < key_num) {
      result[cnt++] = index.leaf_level_.GetKV(lb++);
    }
    return cnt;
  }

  long long memory_consumption() { return index.IndexSize(); }

 private:
  fh_index_ro::FHIndexRO<uint64_t, kTopLevelThreshold, kBlockSize,
                         kFingerDensity, kInternalBlockSize,
                         kInternalFingerDensity>
      index;
};