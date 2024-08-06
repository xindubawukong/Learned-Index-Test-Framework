#include "../indexInterface.h"
#include "./src/structures/arttree/ordered_map.h"
#include "parlay/parallel.h"

template <class KEY_TYPE, class PAYLOAD_TYPE>
class VerlibARTTreeInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
public:
  static_assert(std::is_same_v<KEY_TYPE, uint64_t>,
                "FH_index_RO must have uint64_t key type");

  void init(Param *param = nullptr) {}

  void bulk_load(std::pair<KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,
                 Param *param = nullptr) {
    parlay::parallel_for(0, num, [&](size_t i) {
      map.insert(key_value[i].first, key_value[i].second);
    });
  }

  bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr) {
    auto res = map.find_(key);
    if (res.has_value()) {
      val = res.value();
      return true;
    } else {
      return false;
    }
  }

  bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {
    return map.insert(key, value);
  }

  bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {}

  bool remove(KEY_TYPE key, Param *param = nullptr) {}

  size_t scan(KEY_TYPE key_low_bound, size_t key_num,
              std::pair<KEY_TYPE, PAYLOAD_TYPE> *result,
              Param *param = nullptr) {
    size_t cnt = 0;
    auto f = [&](KEY_TYPE key, PAYLOAD_TYPE value) {
      result[cnt++] = {key, value};
    };
    map.range_scan_(f, key_low_bound, key_num);
    return cnt;
  }

  long long memory_consumption() {}

private:
  verlib_arttree::ordered_map<KEY_TYPE, PAYLOAD_TYPE> map;
};