#include "../competitor/indexInterface.h"
#include <iostream>

// This index does nothing.
// Only used to record memory consumption
template <class KEY_TYPE, class PAYLOAD_TYPE>
class EmptyIndexInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
public:
  void init(Param *param = nullptr) {}

  void bulk_load(std::pair<KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,
                 Param *param = nullptr) {
    std::cout << "empty bulk_load" << std::endl;
  }

  bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr) {
    return false;
  }

  bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {
    return false;
  }

  bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {}

  bool remove(KEY_TYPE key, Param *param = nullptr) {}

  size_t scan(KEY_TYPE key_low_bound, size_t key_num,
              std::pair<KEY_TYPE, PAYLOAD_TYPE> *result,
              Param *param = nullptr) {}

  long long memory_consumption() {}
};