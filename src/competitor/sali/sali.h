#include "../indexInterface.h"
#include "./src/src/core/sali.h"

template <class KEY_TYPE, class PAYLOAD_TYPE>
class SaliInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
public:
  void init(Param *param = nullptr) {}

  void bulk_load(std::pair<KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,
                 Param *param = nullptr) {
    index.bulk_load(key_value, num);
  }

  bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr) {
    return index.at(key, val);
  }

  bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {
    index.insert(key, value);
    return true;
  }

  bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr) {
    return index.update(key, value);
  }

  bool remove(KEY_TYPE key, Param *param = nullptr) {
    return index.remove(key);
  }

  size_t scan(KEY_TYPE key_low_bound, size_t key_num,
              std::pair<KEY_TYPE, PAYLOAD_TYPE> *result,
              Param *param = nullptr) {
    assert(0);
  }

  long long memory_consumption() { return index.total_size(); }

private:
  sali::SALI<KEY_TYPE, PAYLOAD_TYPE> index;
};
