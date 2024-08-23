#ifndef FH_INDEX_RW_INTERFACE_H_
#define FH_INDEX_RW_INTERFACE_H_

#include "../competitor/indexInterface.h"
#include "./fh_index_utils.h"
#include "fh_index_rw/fh_index_rw.h"

template <class KEY_TYPE, class PAYLOAD_TYPE>
class FHIndexRWInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
 public:
  static_assert(std::is_same_v<KEY_TYPE, uint64_t>,
                "FH_index_RW must have uint64_t key type");

  void init(Param *param = nullptr) {}

  void bulk_load(std::pair<KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,
                 Param *param = nullptr) {
    index.BulkLoad(
        num, [&](size_t i) { return key_value[i].first; },
        [&](size_t i) { return key_value[i].second; });
  }

  bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr) {
    auto val_opt = index.Find(key);
    if (val_opt.has_value()) {
      val = val_opt.value();
      return true;
    } else {
      return false;
    }
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
              Param *param = nullptr) {}

  long long memory_consumption() { assert(0); }

 private:
  fh_index_rw::FHIndexRW<uint64_t, kTopLevelThreshold, kSizeLowerLeaf,
                         kSizeUpperLeaf, kFingerPerBlockLeaf,
                         kSizeLowerInternal, kSizeUpperInternal,
                         kFingerPerBlockInternal>
      index;
};

#endif  // FH_INDEX_RW_INTERFACE_H_
