#include "../competitor/indexInterface.h"
#include <cassert>
#include <map>

template <class KEY_TYPE, class PAYLOAD_TYPE>
class NaiveIndexInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
public:
  static_assert(std::is_same_v<KEY_TYPE, uint64_t>,
                "FH_index_RO must have uint64_t key type");

  void init(Param *param = nullptr) {}

  void bulk_load(std::pair<KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,
                 Param *param = nullptr) {
    map = new std::map<KEY_TYPE, PAYLOAD_TYPE>(key_value, key_value + num);
  }

  bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr) {
    auto it = map->find(key);
    if (it == map->end())
      return false;
    val = it->second;
    return true;
  }

  bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {
    (*map)[key] = value;
    return true;
  }

  bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {
    (*map)[key] = value;
    return true;
  }

  bool remove(KEY_TYPE key, Param *param = nullptr) {
    auto it = map->find(key);
    if (it != map->end()) {
      map->erase(it);
    }
    return true;
  }

  size_t scan(KEY_TYPE key_low_bound, size_t key_num,
              std::pair<KEY_TYPE, PAYLOAD_TYPE> *result,
              Param *param = nullptr) {
    auto it = map->lower_bound(key_low_bound);
    size_t cnt = 0;
    while (it != map->end() && cnt < key_num) {
      result[cnt++] = *it;
      it++;
    }
    return cnt;
  }

  long long memory_consumption() { assert(0); }

  ~NaiveIndexInterface() { delete map; }

private:
  std::map<KEY_TYPE, PAYLOAD_TYPE> *map;
};
